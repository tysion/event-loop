#include "event_loop_context.h"

#include <utility>

namespace oxm {

namespace {

void ClearBit(oxm::Event::Mask& mask, oxm::Event::Type event_type) {
  mask &= ~(1 << (static_cast<uint32_t>(event_type) - 1));
}

bool CheckMask(oxm::Event::Mask mask) {
  ClearBit(mask, oxm::Event::Type::Read);
  ClearBit(mask, oxm::Event::Type::Write);
  ClearBit(mask, oxm::Event::Type::FileDescriptorError);
  ClearBit(mask, oxm::Event::Type::RemoteConnectionClosed);
  return mask == 0;
}
}  // namespace

TaskPtr EventLoopContext::CreateTask(Callback&& callback) {
  return std::make_shared<Task>(std::move(callback));
}

void EventLoopContext::Poll(int timeout) {
  notificator_->Wait(timeout, &ready_event_ids_);

  for (const auto& [mask, id] : ready_event_ids_) {
    const auto& [event, task] = event_binds_[id];

    task->Execute(mask);
  }

  ready_event_ids_.clear();
}

Event::Id EventLoopContext::RegisterEvent(Event event) {
  if (event.fd < 0) {
    throw std::invalid_argument("invalid file descriptor");
  }

  if (!CheckMask(event.mask)) {
    throw std::invalid_argument("invalid mask");
  }

  event_binds_.emplace_back(event, nullptr);
  return event_binds_.size() - 1;
}

void EventLoopContext::Bind(Event::Id id, TaskPtr task) {
  if (task == nullptr) {
    throw std::invalid_argument("unable to bind nullptr");
  }
  GetEventBindById(id).second = std::move(task);
}

void EventLoopContext::Schedule(Event::Id id) {
  const auto& [event, task] = GetEventBindById(id);
  if (task == nullptr) {
    throw std::logic_error("unable to schedule event with no bound task");
  }

  notificator_->Watch(event.fd, event.mask, id);
}

void EventLoopContext::Unshedule(Event::Id id, bool forever) {
  auto& [event, task] = GetEventBindById(id);
  if (forever) {
    task.reset();
  }

  notificator_->Unwatch(event.fd);
}

std::pair<Event, TaskPtr>& EventLoopContext::GetEventBindById(oxm::Event::Id id) {
  if (id >= event_binds_.size()) {
    throw std::invalid_argument("invalid event id");
  }
  return event_binds_[id];
}

}  // namespace oxm