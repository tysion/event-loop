#include "event_loop.h"

#include "epoll_notificator.h"
#include "event_loop_context.h"

namespace oxm {

EventLoop::EventLoop() {
  auto notifier = std::make_unique<EpollNotificator>();
  ctx_ = std::make_unique<EventLoopContext>(std::move(notifier));
}

void EventLoop::ExecuteWhen(int fd, EventType event, Callback cb, void* user_data) {
  ctx_->Add(fd, event, cb, user_data);
}

void EventLoop::Poll(int timeout) {
  ctx_->Poll(timeout);
}

EventLoop::~EventLoop() = default;

}  // namespace oxm
