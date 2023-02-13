#include "registry.h"

namespace oxm {

Event::Id Registry::Add(Event event) {
  // TODO: insert into the map
  return 0;
}

std::optional<Registry::MapIt> Registry::Find(Event::Id id) {
  // TODO: find element iterator by id
  return std::nullopt;
}

EventBinding& Registry::ExtractEventBinding(Registry::MapIt it) {
  return it->second;
}

void Registry::Bind(Registry::MapIt it, TaskPtr task) {
  it->second.task = std::move(task);
}

void Registry::Remove(Registry::MapIt it) {
  // TODO: remove from table by iterator
}

}  // namespace oxm
