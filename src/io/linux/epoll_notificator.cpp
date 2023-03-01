#include "epoll_notificator.h"

#include <cassert>

namespace oxm {

uint32_t ToEpollEvent(Event::Mask mask) {
  uint32_t epoll_event = 0;

  if (mask.Has(Event::Type::Read)) {
    epoll_event |= EPOLLIN;
  }

  if (mask.Has(Event::Type::Write)) {
    epoll_event |= EPOLLOUT;
  }

  return epoll_event;
}

Event::Mask ToEventMask(uint32_t events) {
  Event::Mask mask = {};

  if (events & EPOLLERR) {
    mask.Set(Event::Type::FileDescriptorError);
  }

  if (events & EPOLLHUP) {
    mask.Set(Event::Type::RemoteConnectionClosed);
  }

  if (events & EPOLLIN) {
    mask.Set(Event::Type::Read);
  }

  if (events & EPOLLOUT) {
    mask.Set(Event::Type::Write);
  }

  return mask;
}

EpollNotificator::EpollNotificator(int approximate_events_count) {
  events_.resize(approximate_events_count);

  epfd_ = epoll_create1(EPOLL_CLOEXEC);
  if (epfd_ < 0) {
    throw std::runtime_error("failed to epoll_create1");
  }
}

void EpollNotificator::Control(int cmd, int fd, Event::Mask mask, Event::Id id) {
  epoll_event ev{};
  ev.data.u64 = id;
  ev.events = ToEpollEvent(mask);

  int err = epoll_ctl(epfd_, cmd, fd, &ev);
  if (err < 0) {
    throw std::runtime_error("failed to epoll_ctl");
  }
}

void EpollNotificator::WatchImpl(int fd, Event::Mask mask, Event::Id id) {
  Control(EPOLL_CTL_ADD, fd, mask, id);
  ++events_count_;
}

void EpollNotificator::ModifyImpl(int fd, Event::Mask mask) {
  Control(EPOLL_CTL_MOD, fd, mask, Event::Id{});
}

void EpollNotificator::UnwatchImpl(int fd) {
  Control(EPOLL_CTL_DEL, fd, Event::Mask{}, Event::Id{});
  --events_count_;
}

void EpollNotificator::WaitImpl(int timeout, EventIds* ready_event_ids) {
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
    ready_event_ids->push_back({ToEventMask(event.events), event.data.u64});
  }
}

}  // namespace oxm
