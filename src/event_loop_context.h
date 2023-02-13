#pragma once

#include <memory>
#include <vector>

#include "event_notificator_interface.h"
#include "registry.h"
#include "task.h"

namespace oxm {

struct EventLoopContext {
  explicit EventLoopContext(std::unique_ptr<IEventNotificator> notificator)
      : notificator_{std::move(notificator)} {
  }

  TaskPtr CreateTask(Callback&& callback);

  Event::Id RegisterEvent(Event event);

  void Schedule(Event::Id id);

  void Unshedule(Event::Id id, bool forever);

  void Bind(Event::Id id, TaskPtr task);

  void Poll(int timeout = -1);

 private:
  Registry::MapIt GetRegistryIterator(oxm::Event::Id id);

  std::unique_ptr<IEventNotificator> notificator_;
  EventIds ready_event_ids_;
  Registry registry_;
};

}  // namespace oxm