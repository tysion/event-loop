#pragma once

#include <functional>
#include <memory>

namespace oxm {

struct EventLoop;
struct EventLoopContext;

enum class EventType {
  ReadyToReadFrom,
  ReadyToWriteTo
};

struct Task;
using TaskPtr = std::shared_ptr<Task>;

struct Event {
  int fd;
  EventType type;
};

using EventId = size_t;

enum class Status {
  Ok,
  Error
};

using Callback = std::function<void(Status)>;

struct EventLoop {
  EventLoop();

  ~EventLoop();

  TaskPtr CreateTask(Callback&& callback);

  EventId RegisterEvent(Event event);

  void Schedule(EventId id);

  void Bind(EventId id, TaskPtr task);

  /**
   * Wait for events and executes callbacks
   * @param timeout the maximum wait time in milliseconds (-1 == infinite)
   */
  void Poll(int timeout = -1);

 private:
  std::unique_ptr<EventLoopContext> ctx_;
};

}  // namespace oxm
