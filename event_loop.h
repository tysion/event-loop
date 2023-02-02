#pragma once

#include <memory>

namespace oxm {

struct EventLoop;
struct EventLoopContext;

using Callback = void (*)(EventLoop* event_loop, void* user_data);

enum class EventType {
  ReadyToReadFrom,
  ReadyToWriteTo
};

struct EventLoop {
  EventLoop();

  ~EventLoop();

  template <typename F>
  void ExecuteWhen(int fd, EventType event_type, F func) {
    struct Wrapper {
      F func;
      static void Call(EventLoop* event_loop, void* self) {
        static_cast<const Wrapper*>(self)->func(event_loop);
      }
    };

    auto* wrapper = new Wrapper{std::move(func)};

    ExecuteWhen(fd, event_type, Wrapper::Call, wrapper);
  }

  /**
   * Executes user provided callback when an event occurs for the passed file descriptor
   * @param fd file descriptor to watch for
   * @param event event, @see Event
   * @param cb user callback
   * @param user_data user data, that will be passed into the callback
   */
  void ExecuteWhen(int fd, EventType event, Callback cb, void* user_data);

  /**
   * Wait for events and executes callbacks
   * @param timeout the maximum wait time in milliseconds (-1 == infinite)
   */
  void Poll(int timeout = -1);

 private:
  std::unique_ptr<EventLoopContext> ctx_;
};

}  // namespace oxm
