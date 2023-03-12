#include "oxm/event_loop.h"

#include "event_loop_context.h"

namespace oxm {

#define ctx_ptr reinterpret_cast<Context*>(&ctx_)

EventLoop::EventLoop(const Options& options) {
    new(ctx_ptr) Context(options);
}

void EventLoop::Poll(int timeout) {
    ctx_ptr->Poll(timeout);
}

Event::Id EventLoop::RegisterEvent(Event event) {
  return ctx_ptr->RegisterEvent(event);
}

void EventLoop::Schedule(Event::Id id) {
  return ctx_ptr->Schedule(id);
}

void EventLoop::Unschedule(Event::Id id, bool forever) {
  ctx_ptr->Unschedule(id, forever);
}

void EventLoop::Bind(Event::Id id, Task* task) {
  ctx_ptr->Bind(id, task);
}

Task* EventLoop::AllocateTask(size_t task_size) {
  return ctx_ptr->AllocateTask(task_size);
}

EventLoop::~EventLoop() {
  ctx_ptr->~Context();
}

}  // namespace oxm
