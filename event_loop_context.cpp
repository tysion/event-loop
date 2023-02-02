#include "event_loop_context.h"

namespace oxm {

EventCallback& EventLoopContext::GetEventCallbackById(EventId id) {
  return callbacks_[id];
}

EventId EventLoopContext::EmplaceEventCallback(Callback cb, void* user_data) {
  callbacks_.emplace_back(cb, user_data);
  return callbacks_.size() - 1;
}

void EventLoopContext::Add(int fd, EventType event, Callback cb, void* user_data) {
  const auto id = EmplaceEventCallback(cb, user_data);
  notificator_->Watch(fd, event, id);
}

void EventLoopContext::Remove(int fd) {
  notificator_->Unwatch(fd);
}

void EventLoopContext::Poll(int timeout) {
  notificator_->ListReadyEventIds(timeout, &ready_event_ids_);

  for (const EventId id : ready_event_ids_) {
    auto& callback = GetEventCallbackById(id);

    callback.Call(event_loop_);
  }

  ready_event_ids_.clear();
}

}  // namespace oxm