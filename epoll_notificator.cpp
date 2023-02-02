#include "epoll_notificator.h"

#include <cassert>

namespace oxm {

int ToEpollEvent(EventType event_type) {
  switch (event_type) {
    case EventType::ReadyToReadFrom:
      return EPOLLIN | EPOLLHUP | EPOLLERR;
    case EventType::ReadyToWriteTo:
      return EPOLLOUT | EPOLLHUP | EPOLLERR;
    default:
      assert(false); return -1;
  }
}

EpollNotificator::EpollNotificator(int approximate_events_count) {
  events_.resize(approximate_events_count);

  epfd_ = epoll_create1(EPOLL_CLOEXEC);
  if (epfd_ < 0) {
    throw std::runtime_error("failed to epoll_create1");
  }
}

void EpollNotificator::Control(int cmd, int fd, EventType type, EventId id) {
  epoll_event ev{};
  ev.data.u64 = id;
  ev.events = ToEpollEvent(type);

  int err = epoll_ctl(epfd_, cmd, fd, &ev);
  if (err < 0) {
    throw std::runtime_error("failed to epoll_ctl");
  }
}

void EpollNotificator::Watch(int fd, EventType type, EventId id) {
  Control(EPOLL_CTL_ADD, fd, type, id);
  ++events_count_;
}

void EpollNotificator::Update(int fd, EventType type) {
  Control(EPOLL_CTL_MOD, fd, type, EventId{});
}

void EpollNotificator::Unwatch(int fd) {
  Control(EPOLL_CTL_DEL, fd, EventType{}, EventId{});
  --events_count_;
}

void EpollNotificator::ListReadyEventIds(int timeout, std::vector<EventId>* ready_event_ids) {
  assert(ready_event_ids);
  assert(ready_event_ids->empty());

  events_.resize(events_count_);
  const int num_ready_events = epoll_wait(epfd_, events_.data(), events_.size(), timeout);
  if (num_ready_events < 0) {
    throw std::runtime_error("failed to epoll_wait");
  }

  ready_event_ids->reserve(num_ready_events);
  for (int i = 0; i < num_ready_events; ++i) {
    const auto& event = events_[i];
    ready_event_ids->push_back(event.data.u64);
  }
}

}  // namespace oxm
