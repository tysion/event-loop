#pragma once

#include <functional>
#include <memory>

#include "oxm/event.h"
#include "oxm/task.h"
#include "oxm/executor.h"

namespace oxm {

struct Options {
  size_t number_events_per_poll = 1024;
  size_t task_allocator_buffer_size = 32 * 1024;
  size_t num_worker_threads = 4;
  size_t worker_thread_queue_size = 128;
  ExecutorPtr executor = nullptr;
};

struct EventLoop {
  EventLoop(const Options& options = {});
  ~EventLoop();

  EventLoop(const EventLoop& other) = delete;
  EventLoop& operator=(const EventLoop& other) = delete;

  template <typename T>
  Event::Id Submit(Event event, T callback) {
    const Event::Id id = RegisterEvent(event);
    Bind(id, CreateTask(std::move(callback)));
    return id;
  }

  void Schedule(Event::Id id);

  void Unschedule(Event::Id id, bool forever);

  /**
   * wait_fn for events and executes callbacks
   * @param timeout the maximum wait time in milliseconds (-1 == infinite)
   */
  void Poll(int timeout = -1);

 private:
  Task* AllocateTask(size_t task_size);

  template <typename T>
  Task* CreateTask(T callback) {
    struct TaskWrapper final : Task {
      explicit TaskWrapper(T&& callback) : callback_{std::move(callback)} {
      }

      void Execute(oxm::Event::Mask mask) final {
        callback_(mask);
      }

     private:
      T callback_;
    };

    return new (AllocateTask(sizeof(TaskWrapper))) TaskWrapper(std::move(callback));
  }

  void Bind(Event::Id id, Task* task);

  Event::Id RegisterEvent(Event event);

  static constexpr size_t kContextSize = 168;
  static constexpr size_t kContextAlignment = 8;
  std::aligned_storage_t<kContextSize, kContextAlignment> ctx_;
};

}  // namespace oxm
