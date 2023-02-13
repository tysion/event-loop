#pragma once

#include <optional>
#include <unordered_map>

#include "task.h"

namespace oxm {

struct EventBinding {
  Event event;
  TaskPtr task;
};

struct Registry {
  using Map = std::unordered_map<Event::Id, EventBinding>;
  using MapIt = Map::iterator;

  /// Adds an event to the registry and assigns an ID to it
  /// @return ID of the added event
  Event::Id Add(Event event);

  /// Finds a binding
  /// @param id ID of an element
  /// @return empty optional if ID is not existed, otherwise valid map iterator
  std::optional<MapIt> Find(Event::Id id);

  /// Extracts event binding from map iterator
  /// @param it map iterator
  /// @return reference to corresponding event binding
  static EventBinding& ExtractEventBinding(MapIt it);

  /// Binds a task to an existing event
  /// @param it map iterator
  /// @param task binding task
  static void Bind(MapIt it, TaskPtr task);

  /// Removes an event from the registry
  /// @param it map iterator
  /// @note iterator invalidates after removing an element
  void Remove(MapIt it);

 private:
  Event::Id id_ = 0;
  std::unordered_map<Event::Id, EventBinding> map_;
};

}  // namespace oxm
