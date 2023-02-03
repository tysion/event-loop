#include "event_loop_context.h"

#include <utility>

namespace oxm {

TaskPtr EventLoopContext::CreateTask(Callback&& callback) {
  return std::make_shared<Task>(std::move(callback));
}

void EventLoopContext::Poll(int timeout) {
  notificator_->ListReadyEventIds(timeout, &ready_event_ids_);

  for (const auto& [status, id]: ready_event_ids_) {
    const auto& [event, task] = bound_events_[id];

    notificator_->Unwatch(event.fd);

    task->Execute(status);
  }

  ready_event_ids_.clear();
}

EventId EventLoopContext::RegisterEvent(Event event) {
  bound_events_.emplace_back(event, nullptr);
  return bound_events_.size() - 1;
}

void EventLoopContext::Bind(EventId id, TaskPtr task) {
  bound_events_[id].second = std::move(task);
}

void EventLoopContext::Schedule(EventId id) {
  const auto& [event, task] = bound_events_[id];
  if (task == nullptr) {
    throw std::logic_error("unable to schedule event with no bound task");
  }

  notificator_->Watch(event.fd, event.type, id);
}

}  // namespace oxm