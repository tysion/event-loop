#include "oxm/event_loop.h"

#include "event_loop_context.h"

namespace oxm {

EventLoop::EventLoop() {
  ctx_ = std::make_unique<Context>();
}

void EventLoop::Poll(int timeout) {
  ctx_->impl.Poll(timeout);
}

Event::Id EventLoop::RegisterEvent(Event event) {
  return ctx_->impl.RegisterEvent(event);
}

void EventLoop::Schedule(Event::Id id) {
  return ctx_->impl.Schedule(id);
}

void EventLoop::Unshedule(Event::Id id, bool forever) {
  ctx_->impl.Unshedule(id, forever);
}

void EventLoop::Bind(Event::Id id, Task* task) {
  ctx_->impl.Bind(id, task);
}

Task* EventLoop::AllocateTask(size_t task_size) {
  return ctx_->impl.AllocateTask(task_size);
}

EventLoop::~EventLoop() = default;

}  // namespace oxm
