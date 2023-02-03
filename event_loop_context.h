#pragma once

#include <memory>
#include <vector>

#include "event_notificator_interface.h"
#include "task.h"

namespace oxm {

struct EventLoopContext;

struct EventLoopContext {
  explicit EventLoopContext(std::unique_ptr<IEventNotificator> notificator)
      : notificator_{std::move(notificator)}
  {}

  TaskPtr CreateTask(Callback&& callback);

  EventId RegisterEvent(Event event);

  void Schedule(EventId id);

  void Bind(EventId id, TaskPtr task);

  void Poll(int timeout = -1);

 private:
  std::unique_ptr<IEventNotificator> notificator_;
  std::vector<std::pair<Status, EventId>> ready_event_ids_;
  std::vector<std::pair<Event, TaskPtr>> bound_events_;
};

}  // namespace oxm