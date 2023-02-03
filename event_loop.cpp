#include "event_loop.h"

#include "epoll_notificator.h"
#include "event_loop_context.h"

namespace oxm {

EventLoop::EventLoop() {
  auto notifier = std::make_unique<EpollNotificator>();
  ctx_ = std::make_unique<EventLoopContext>(std::move(notifier));
}

void EventLoop::Poll(int timeout) {
  ctx_->Poll(timeout);
}

TaskPtr EventLoop::CreateTask(Callback&& callback) {
  return ctx_->CreateTask(std::move(callback));
}

Event::Id EventLoop::RegisterEvent(Event event) {
  return ctx_->RegisterEvent(event);
}

void EventLoop::Schedule(Event::Id id) {
  return ctx_->Schedule(id);
}

void EventLoop::Bind(Event::Id id, TaskPtr task) {
  ctx_->Bind(id, task);
}

EventLoop::~EventLoop() = default;

}  // namespace oxm
