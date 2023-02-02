#pragma once

#include <sys/epoll.h>

#include <vector>

#include "event_notificator_interface.h"

namespace oxm {

struct EpollNotificator final : IEventNotificator {
  explicit EpollNotificator(int approximate_events_count = 64);

  void Watch(int fd, EventType type, EventId id) final;

  void Update(int fd, EventType type) final;

  void Unwatch(int fd) final;

  void ListReadyEventIds(int timeout, std::vector<EventId>* ready_event_ids) final;

 private:

  void Control(int cmd, int fd, EventType type, EventId id = 0);

  int epfd_;
  size_t events_count_ = 0;
  std::vector<epoll_event> events_;
};

}  // namespace oxm
