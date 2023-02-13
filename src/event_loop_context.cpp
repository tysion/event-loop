#include "event_loop_context.h"

#include <utility>

namespace oxm {

TaskPtr EventLoopContext::CreateTask(Callback&& callback) {
  return std::make_shared<Task>(std::move(callback));
}

void EventLoopContext::Poll(int timeout) {
  notificator_->Wait(timeout, &ready_event_ids_);

  for (const auto& [mask, id] : ready_event_ids_) {
    auto it = registry_.Find(id);
    if (!it) {
      throw std::logic_error("invalid event id");
    }

    const auto& [event, task] = Registry::ExtractEventBinding(*it);

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

  return registry_.Add(event);
}

void EventLoopContext::Bind(Event::Id id, TaskPtr task) {
  if (task == nullptr) {
    throw std::invalid_argument("task is nullptr");
  }

  auto it = GetRegistryIterator(id);

  Registry::Bind(it, std::move(task));
}

void EventLoopContext::Schedule(Event::Id id) {
  auto it = GetRegistryIterator(id);
  const auto& [event, task] = Registry::ExtractEventBinding(it);

  if (task == nullptr) {
    throw std::logic_error("unable to schedule event with no bound task");
  }

  notificator_->Watch(event.fd, event.mask, id);
}

void EventLoopContext::Unshedule(Event::Id id, bool forever) {
  auto it = GetRegistryIterator(id);
  const auto& [event, task] = Registry::ExtractEventBinding(it);

  notificator_->Unwatch(event.fd);

  if (forever) {
    registry_.Remove(it);
  }
}

Registry::MapIt EventLoopContext::GetRegistryIterator(oxm::Event::Id id) {
  auto it = registry_.Find(id);
  if (!it) {
    throw std::invalid_argument("invalid event id");
  }

  return *it;
}

}  // namespace oxm