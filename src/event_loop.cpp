#include <utility>

#include "oxm/event_loop.h"

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

Event::Id EventLoop::RegisterEvent(Event event) {
  return ctx_->RegisterEvent(event);
}

void EventLoop::Schedule(Event::Id id) {
  return ctx_->Schedule(id);
}

void EventLoop::Unshedule(Event::Id id, bool forever) {
  ctx_->Unshedule(id, forever);
}

void EventLoop::Bind(Event::Id id, Task* task) {
  ctx_->Bind(id, task);
}

Task* EventLoop::AllocateTask(size_t task_size) {
  return ctx_->AllocateTask(task_size);
}

EventLoop::~EventLoop() = default;

}  // namespace oxm
