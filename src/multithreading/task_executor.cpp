#include "task_executor.h"

namespace oxm {

bool TaskExecutor::TryPopTaskFromLocalQueue(TaskPackage* task) {
  return local_work_queue_ && local_work_queue_->TryPop(*task);
}

bool TaskExecutor::TryPopTaskFromPoolQueue(TaskPackage* task) {
  return pool_work_queue_.TryPop(*task);
}

bool TaskExecutor::TryPopTaskFromOtherThreadQueue(TaskPackage* task) {
  for (uint32_t i = 0; i < queues_.size(); ++i) {
    auto index = (worker_id_ + i + 1) % queues_.size();
    if (queues_[index]->TryPop(*task)) {
      return true;
    }
  }

  return false;
}

void TaskExecutor::WorkerThread(uint32_t thread_id) {
  worker_id_ = thread_id;
  local_work_queue_ = queues_[worker_id_].get();
  while (!done_) {
    RunPendingTask();
  }
}

void TaskExecutor::RunPendingTask() {
  TaskPackage package;
  if (TryPopTaskFromLocalQueue(&package) || TryPopTaskFromPoolQueue(&package) ||
      TryPopTaskFromOtherThreadQueue(&package)) {
    ExecuteTaskPackage(std::move(package));
  } else {
    std::this_thread::yield();
  }
}

void TaskExecutor::PushTask(Task* task, oxm::Event::Mask mask) {
  TaskPackage package = {task, mask};

  if (local_work_queue_) {
    WaitPush(local_work_queue_, std::move(package));
    return;
  }

  WaitPush(&pool_work_queue_, std::move(package));
}

void TaskExecutor::ExecuteTaskPackage(TaskExecutor::TaskPackage&& package) {
  package.first->Execute(package.second);
}

void TaskExecutor::WaitPush(LockFreeQueue<TaskPackage>* queue, TaskPackage&& package) {
  while (!queue->TryPush(package)) {
    RunPendingTask();
  }
}

TaskExecutor::~TaskExecutor() {
  done_ = true;
  for (auto& thread : threads_) {
    thread.join();
  }
}

TaskExecutor::TaskExecutor(uint32_t num_threads, uint32_t work_queue_capacity)
    : done_(false), pool_work_queue_(work_queue_capacity) {
  if (num_threads == 0) {
    num_threads = std::thread::hardware_concurrency() - 1;
  }

  try {
    queues_.reserve(num_threads);
    threads_.reserve(num_threads);
    for (uint32_t i = 0; i < num_threads; ++i) {
      queues_.push_back(std::make_unique<WorkQueue>(work_queue_capacity));
      threads_.emplace_back(&TaskExecutor::WorkerThread, this, i);
    }
  } catch (...) {
    done_ = true;
    throw;
  }
}

void TaskExecutor::Wait(Predicate predicate, void* user_data) {
  while (!predicate(user_data)) {
    RunPendingTask();
  }
}

void PushTask(Task* task, oxm::Event::Mask mask, void* ctx) {
  static_cast<TaskExecutor*>(ctx)->PushTask(task, mask);
}

void Wait(Predicate predicate, void* user_data, void* ctx) {
  static_cast<TaskExecutor*>(ctx)->Wait(predicate, user_data);
}

void Terminate(void* ctx) {
  delete static_cast<TaskExecutor*>(ctx);
}

std::shared_ptr<Executor> MakeExecutor(uint32_t num_threads, uint32_t work_queue_capacity) {
  auto executor = std::make_shared<Executor>();
  executor->ctx = new TaskExecutor(num_threads, work_queue_capacity);
  executor->push_task_fn = PushTask;
  executor->wait_fn = Wait;
  executor->terminate_fn = Terminate;
  return executor;
}

}  // namespace oxm
