#pragma once

#include <memory>
#include <mutex>
#include <vector>

#include "io/notificator.h"
#include "multithreading/async_mutex.h"
#include "multithreading/task_executor.h"
#include "oxm/task.h"
#include "task_allocator.h"

namespace oxm {

template <typename TNotificator>
struct EventLoopContext {
  explicit EventLoopContext(const Options& options = {})
      : notificator_{options.number_events_per_poll},
        allocator_{options.task_allocator_buffer_size},
        executor_{options.executor
                      ? options.executor
                      : MakeExecutor(options.num_worker_threads, options.worker_thread_queue_size)},
        mutex_{executor_} {
  }

  Task* AllocateTask(size_t task_size);

  Event::Id RegisterEvent(Event event);

  void Schedule(Event::Id id);

  void Unschedule(Event::Id id, bool forever);

  void Bind(Event::Id id, Task* task);

  void Poll(int timeout = -1);

  const TNotificator& GetNotificator() const {
    return notificator_;
  }

 private:
  std::pair<Event, Task*>& GetEventBindById(oxm::Event::Id id);

  void DeallocateTask(Task* task);

  TNotificator notificator_;
  EventIds ready_event_ids_;
  std::vector<std::pair<Event, Task*>> event_binds_;

  TaskAllocator allocator_;
  ExecutorPtr executor_;

  AsyncMutex mutex_;
};

struct Context {
  Context(const Options& options) : impl{options} {
  }

  EventLoopContext<Notificator> impl;
};

template <typename TNotificator>
Task* EventLoopContext<TNotificator>::AllocateTask(size_t task_size) {
  auto lock = std::lock_guard(mutex_);
  return allocator_.Allocate(task_size);
}

template <typename TNotificator>
void EventLoopContext<TNotificator>::Poll(int timeout) {
  auto lock = std::lock_guard(mutex_);
  notificator_.Wait(timeout, &ready_event_ids_);

  for (const auto& [mask, id] : ready_event_ids_) {
    const auto& [event, task] = event_binds_[id];
    executor_->PushTask(task, mask);
  }

  ready_event_ids_.clear();
}

template <typename TNotificator>
Event::Id EventLoopContext<TNotificator>::RegisterEvent(Event event) {
  auto lock = std::lock_guard(mutex_);

  if (event.fd < 0) {
    throw std::invalid_argument("invalid file descriptor");
  }

  if (!event.mask.IsValid()) {
    throw std::invalid_argument("invalid mask");
  }

  event_binds_.emplace_back(event, nullptr);
  return event_binds_.size() - 1;
}

template <typename TNotificator>
void EventLoopContext<TNotificator>::Bind(Event::Id id, Task* task) {
  auto lock = std::lock_guard(mutex_);

  if (task == nullptr) {
    throw std::invalid_argument("unable to bind nullptr");
  }
  GetEventBindById(id).second = task;
}

template <typename TNotificator>
void EventLoopContext<TNotificator>::Schedule(Event::Id id) {
  auto lock = std::lock_guard(mutex_);

  const auto& [event, task] = GetEventBindById(id);
  if (task == nullptr) {
    throw std::logic_error("unable to schedule event with no bound task");
  }

  notificator_.Watch(event.fd, event.mask, id);
}

template <typename TNotificator>
void EventLoopContext<TNotificator>::Unschedule(Event::Id id, bool forever) {
  auto lock = std::lock_guard(mutex_);
  auto& [event, task] = GetEventBindById(id);

  if (forever) {
    DeallocateTask(task);
  }

  notificator_.Unwatch(event.fd);
}

template <typename TNotificator>
void EventLoopContext<TNotificator>::DeallocateTask(Task* task) {
  allocator_.Deallocate(task);
}

template <typename TNotificator>
std::pair<Event, Task*>& EventLoopContext<TNotificator>::GetEventBindById(oxm::Event::Id id) {
  if (id >= event_binds_.size()) {
    throw std::invalid_argument("invalid event id");
  }
  return event_binds_[id];
}

}  // namespace oxm