#pragma once

#include <sys/epoll.h>

#include "io/base_notificator.h"

namespace oxm {

struct EpollNotificator final : BaseNotificator<EpollNotificator> {
  explicit EpollNotificator(int approximate_events_count);

  void WatchImpl(int fd, Event::Mask mask, Event::Id id);

  void ModifyImpl(int fd, Event::Mask mask);

  void UnwatchImpl(int fd);

  void WaitImpl(int timeout, EventIds* ready_event_ids);

  void Control(int cmd, int fd, Event::Mask mask, Event::Id id);

  int epfd_;
  size_t events_count_ = 0;
  std::vector<epoll_event> events_;
};

}  // namespace oxm
