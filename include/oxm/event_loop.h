#pragma once

#include <functional>
#include <memory>

#include "event.h"
#include "task.h"

namespace oxm {

struct Context;
struct Task;

struct EventLoop {
  EventLoop();
  ~EventLoop();

  EventLoop(const EventLoop& other) = delete;
  EventLoop& operator=(const EventLoop& other) = delete;

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

  Event::Id RegisterEvent(Event event);

  void Schedule(Event::Id id);

  void Unschedule(Event::Id id, bool forever);

  void Bind(Event::Id id, Task* task);

  /**
   * Wait for events and executes callbacks
   * @param timeout the maximum wait time in milliseconds (-1 == infinite)
   */
  void Poll(int timeout = -1);

 private:
  Task* AllocateTask(size_t task_size);

  std::unique_ptr<Context> ctx_;
};

}  // namespace oxm
