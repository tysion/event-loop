#pragma once

#include <memory>
#include <vector>

#include "event_notificator_interface.h"
#include "oxm/task.h"
#include "task_allocator.h"

namespace oxm {

struct EventLoopContext {
  explicit EventLoopContext(std::unique_ptr<IEventNotificator> notificator)
      : notificator_{std::move(notificator)}, allocator_{32 * 1024} {
  }

  Task* AllocateTask(size_t task_size);
  void DeallocateTask(Task* task);

  Event::Id RegisterEvent(Event event);

  void Schedule(Event::Id id);

  void Unshedule(Event::Id id, bool forever);

  void Bind(Event::Id id, Task* task);

  void Poll(int timeout = -1);

 private:
  std::pair<Event, Task*>& GetEventBindById(oxm::Event::Id id);

  std::unique_ptr<IEventNotificator> notificator_;
  EventIds ready_event_ids_;
  std::vector<std::pair<Event, Task*>> event_binds_;

  TaskAllocator allocator_;
};

}  // namespace oxm