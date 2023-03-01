#pragma once

#include "oxm/event_loop.h"

namespace oxm {

using EventIds = std::vector<std::pair<Event::Mask, Event::Id>>;

template<typename TNotificator>
struct BaseNotificator {
  void Watch(int fd, Event::Mask mask, Event::Id id) {
    Derived()->WatchImpl(fd, mask, id);
  }

  void Modify(int fd, Event::Mask mask) {
    Derived()->ModifyImpl(fd, mask);
  }

  void Unwatch(int fd) {
    Derived()->UnwatchImpl(fd);
  }

  void Wait(int timeout, EventIds* ready_event_ids) {
    Derived()->WaitImpl(timeout, ready_event_ids);
  }

 private:
  TNotificator* Derived() {
    return static_cast<TNotificator*>(this);
  }
};

}  // namespace oxm
