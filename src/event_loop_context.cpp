#include "event_loop_context.h"

#include <utility>

namespace oxm {

Task* EventLoopContext::AllocateTask(size_t task_size) {
   return allocator_.Allocate(task_size);
}

void EventLoopContext::DeallocateTask(Task* task) {
   allocator_.Deallocate(task);
}

void EventLoopContext::Poll(int timeout) {
  notificator_->Wait(timeout, &ready_event_ids_);

  for (const auto& [mask, id] : ready_event_ids_) {
    const auto& [event, task] = event_binds_[id];

    task->Execute(mask);
  }

  ready_event_ids_.clear();
}

Event::Id EventLoopContext::RegisterEvent(Event event) {
  if (event.fd < 0) {
    throw std::invalid_argument("invalid file descriptor");
  }

  if (!event.mask.IsValid()) {
    throw std::invalid_argument("invalid mask");
  }

  event_binds_.emplace_back(event, nullptr);
  return event_binds_.size() - 1;
}

void EventLoopContext::Bind(Event::Id id, Task* task) {
  if (task == nullptr) {
    throw std::invalid_argument("unable to bind nullptr");
  }
  GetEventBindById(id).second = task;
}

void EventLoopContext::Schedule(Event::Id id) {
  const auto& [event, task] = GetEventBindById(id);
  if (task == nullptr) {
    throw std::logic_error("unable to schedule event with no bound task");
  }

  notificator_->Watch(event.fd, event.mask, id);
}

void EventLoopContext::Unshedule(Event::Id id, bool forever) {
  auto& [event, task] = GetEventBindById(id);
  if (forever) {
    DeallocateTask(task);
  }

  notificator_->Unwatch(event.fd);
}

std::pair<Event, Task*>& EventLoopContext::GetEventBindById(oxm::Event::Id id) {
  if (id >= event_binds_.size()) {
    throw std::invalid_argument("invalid event id");
  }
  return event_binds_[id];
}

}  // namespace oxm