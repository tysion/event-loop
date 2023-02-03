#include "epoll_notificator.h"

#include <cassert>

namespace oxm {

uint32_t ToEpollEvent(Event::Mask mask) {
  uint32_t epoll_event = 0;

  if (mask & Event::Type::Read) {
    epoll_event |= EPOLLIN;
  }

  if (mask & Event::Type::Write) {
    epoll_event |= EPOLLOUT;
  }

  return epoll_event;
}

Event::Mask ToStatus(uint32_t events) {
  Event::Mask mask = Event::Type::None;

  if (events & EPOLLERR) {
    mask |= Event::Type::FileDescriptorError;
  }

  if (events & EPOLLHUP) {
    mask |= Event::Type::RemoteConnectionClosed;
  }

  if (events & EPOLLIN) {
    mask |= Event::Type::Read;
  }

  if (events & EPOLLOUT) {
    mask |= Event::Type::Write;
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

void EpollNotificator::Watch(int fd, Event::Mask mask, Event::Id id) {
  Control(EPOLL_CTL_ADD, fd, mask, id);
  ++events_count_;
}

void EpollNotificator::Update(int fd, Event::Mask mask) {
  Control(EPOLL_CTL_MOD, fd, mask, Event::Id{});
}

void EpollNotificator::Unwatch(int fd) {
  Control(EPOLL_CTL_DEL, fd, Event::Type::None, Event::Id{});
  --events_count_;
}

void EpollNotificator::ListReadyEventIds(int timeout, EventIds* ready_event_ids) {
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
    ready_event_ids->push_back({ToStatus(event.events), event.data.u64});
  }
}

}  // namespace oxm
