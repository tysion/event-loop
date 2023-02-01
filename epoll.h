#pragma once

#include <sys/epoll.h>

#include <stdexcept>
#include <vector>

namespace oxm {

struct Epoll;
struct Task;

using Callback = void (*)(int fd, Epoll* epoll, void* user_data);

enum class EventType {
  ReadyToReadFrom,
  ReadyToWriteTo
};

struct Epoll {
  explicit Epoll(int approximate_events_count = 64);

  template <typename F>
  void ExecuteWhen(int fd, EventType event_type, F func) {
    struct Wrapper {
      F func;
      static void Call(int fd, Epoll* epoll, void* user_data) {
        auto* self = static_cast<const Wrapper*>(user_data);
        self->func(fd, epoll);
        delete self;
      }
    };

    auto* wrapper = new Wrapper{std::move(func)};

    ExecuteWhen(fd, event_type, Wrapper::Call, wrapper);
  }

  void ExecuteWhen(int fd, EventType event_type, Callback cb, void* user_data);

  void Poll();

 private:

  void TrackDescriptor(int fd, epoll_event* ev);

  void UntrackDescriptor(int fd);

  static int ToEpollEvent(EventType event_type);

  int epfd_;
  int events_count_ = 0;
  std::vector<epoll_event> ready_events_;
};

}  // namespace oxm
