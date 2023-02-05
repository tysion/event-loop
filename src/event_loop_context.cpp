#include "event_loop_context.h"

#include <utility>

namespace oxm {

TaskPtr EventLoopContext::CreateTask(Callback&& callback) {
  return std::make_shared<Task>(std::move(callback));
}

void EventLoopContext::Poll(int timeout) {
  notificator_->Wait(timeout, &ready_event_ids_);

  for (const auto& [mask, id]: ready_event_ids_) {
    const auto& [event, task] = event_binds_[id];

    task->Execute(mask);
  }

  ready_event_ids_.clear();
}

Event::Id EventLoopContext::RegisterEvent(Event event) {
  event_binds_.emplace_back(event, nullptr);
  return event_binds_.size() - 1;
}

void EventLoopContext::Bind(Event::Id id, TaskPtr task) {
  if (task == nullptr) {
    throw std::invalid_argument("unable to bind nullptr");
  }
  event_binds_[id].second = std::move(task);
}

void EventLoopContext::Schedule(Event::Id id) {
  const auto& [event, task] = event_binds_.at(id);
  if (task == nullptr) {
    throw std::logic_error("unable to schedule event with no bound task");
  }

  notificator_->Watch(event.fd, event.mask, id);
}

void EventLoopContext::Unshedule(Event::Id id, bool forever) {
  auto& [event, task] = event_binds_.at(id);
  if (forever) {
    task.reset();
  }

  notificator_->Unwatch(event.fd);
}

}  // namespace oxm