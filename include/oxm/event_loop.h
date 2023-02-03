#pragma once

#include <functional>
#include <memory>

#include "event.h"

namespace oxm {

struct EventLoopContext;
struct Task;

using TaskPtr = std::shared_ptr<Task>;
using Callback = std::function<void(Event::Mask)>;

struct EventLoop {
  EventLoop();
  ~EventLoop();

  EventLoop(const EventLoop& other) = delete;
  EventLoop& operator=(const EventLoop& other) = delete;

  TaskPtr CreateTask(Callback&& callback);

  Event::Id RegisterEvent(Event event);

  void Schedule(Event::Id id);

  void Bind(Event::Id id, TaskPtr task);

  /**
   * Wait for events and executes callbacks
   * @param timeout the maximum wait time in milliseconds (-1 == infinite)
   */
  void Poll(int timeout = -1);

 private:
  std::unique_ptr<EventLoopContext> ctx_;
};

}  // namespace oxm
