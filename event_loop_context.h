#pragma once

#include <memory>
#include <vector>

#include "event_notificator_interface.h"

namespace oxm {

struct EventLoopContext;

struct EventCallback {
  EventCallback(Callback cb, void* user_data)
      : cb_{cb}, user_data_{user_data}
  {}

  void Call(EventLoop* event_loop) {
    cb_(event_loop, user_data_);
  }

 private:
  Callback cb_;
  void* user_data_;
};

struct EventLoopContext {
  explicit EventLoopContext(std::unique_ptr<IEventNotificator> notificator)
      : notificator_{std::move(notificator)}
  {}

  void Add(int fd, EventType event, Callback cb, void* user_data);

  void Remove(int fd);

  void Poll(int timeout = -1);

 private:

  EventId EmplaceEventCallback(Callback cb, void* user_data);

  EventCallback& GetEventCallbackById(EventId id);

  EventLoop* event_loop_{};
  std::unique_ptr<IEventNotificator> notificator_;
  std::vector<EventId> ready_event_ids_;
  std::vector<EventCallback> callbacks_; // TODO turn into simple hash map EventId => EventCallback
};

}  // namespace oxm