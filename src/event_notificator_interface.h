#pragma once

#include "oxm/event_loop.h"

namespace oxm {

using EventIds = std::vector<std::pair<Event::Mask, Event::Id>>;

struct IEventNotificator {
  virtual ~IEventNotificator() = default;

  virtual void Watch(int fd, Event::Mask mask, Event::Id id) = 0;

  virtual void Update(int fd, Event::Mask mask) = 0;

  virtual void Unwatch(int fd) = 0;

  virtual void ListReadyEventIds(int timeout, EventIds* ready_event_ids) = 0;
};

}  // namespace oxm
