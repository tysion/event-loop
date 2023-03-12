#pragma once

#include <atomic>
#include <thread>
#include <vector>

#include "lock_free_queue.h"
#include "oxm/executor.h"

namespace oxm {

struct TaskExecutor {
  TaskExecutor(uint32_t num_threads, uint32_t work_queue_capacity = 1024);
  ~TaskExecutor();

  void PushTask(Task* task, oxm::Event::Mask mask);

  void Wait(Predicate predicate, void* user_data);

 private:
  using TaskPackage = std::pair<Task*, oxm::Event::Mask>;
  using WorkQueue = LockFreeQueue<TaskPackage>;

  void WorkerThread(uint32_t thread_id);

  bool TryPopTaskFromLocalQueue(TaskPackage* task);
  bool TryPopTaskFromPoolQueue(TaskPackage* task);
  bool TryPopTaskFromOtherThreadQueue(TaskPackage* task);

  void RunPendingTask();

  static void ExecuteTaskPackage(TaskPackage&& package);
  void WaitPush(LockFreeQueue<TaskPackage>* queue, TaskPackage&& package);

  inline static thread_local WorkQueue* local_work_queue_;
  inline static thread_local uint32_t worker_id_;

  std::atomic_bool done_;
  WorkQueue pool_work_queue_;
  std::vector<std::unique_ptr<WorkQueue>> queues_;
  std::vector<std::thread> threads_;
};

ExecutorPtr MakeExecutor(uint32_t num_threads, uint32_t work_queue_capacity);

}  // namespace oxm
