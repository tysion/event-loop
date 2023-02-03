#pragma once

#include <functional>
#include <memory>

namespace oxm {

struct EventLoop;
struct EventLoopContext;
struct Task;

struct Event {
  using Id = size_t;
  using Mask = uint32_t;

  enum Type : uint32_t {
    None = 0,
    Read = 1 << 0,
    Write = 1 << 1,
    FileDescriptorError = 1 << 2,
    RemoteConnectionClosed = 1 << 3
  };

  void TriggerOn(Type type) {
    mask |= type;
  }

  int fd = -1;
  Mask mask = None;
};

inline bool HasError(Event::Mask mask) {
  return mask & oxm::Event::RemoteConnectionClosed || mask & oxm::Event::FileDescriptorError;
}

inline bool CanRead(Event::Mask mask) {
  return mask & oxm::Event::Read;
}

inline bool CanWrite(Event::Mask mask) {
  return mask & oxm::Event::Write;
}

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
