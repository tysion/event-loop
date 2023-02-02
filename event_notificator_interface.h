#pragma once

#include "event_loop.h"

namespace oxm {

using EventId = size_t;

struct IEventNotificator {
  virtual ~IEventNotificator() = default;

  virtual void Watch(int fd, EventType type, EventId id) = 0;

  virtual void Update(int fd, EventType type) = 0;

  virtual void Unwatch(int fd) = 0;

  virtual void ListReadyEventIds(int timeout, std::vector<EventId>* ready_event_ids) = 0;
};

}  // namespace oxm
