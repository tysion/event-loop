#pragma once

#include <sys/epoll.h>

#include "event_notificator_interface.h"

namespace oxm {

struct EpollNotificator final : IEventNotificator {
  explicit EpollNotificator(int approximate_events_count = 64);

  void Watch(int fd, Event::Mask mask, Event::Id id) final;

  void Modify(int fd, Event::Mask mask) final;

  void Unwatch(int fd) final;

  void Wait(int timeout, EventIds* ready_event_ids) final;

 private:
  void Control(int cmd, int fd, Event::Mask mask, Event::Id id);

  int epfd_;
  size_t events_count_ = 0;
  std::vector<epoll_event> events_;
};

}  // namespace oxm
