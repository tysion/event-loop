#pragma once

#include <memory>

#include "oxm/event.h"

namespace oxm {

struct Task {
  virtual ~Task() = default;

  virtual void Execute(Event::Mask mask) = 0;

  enum class Status {
    None,
    Scheduled,
    InProgress,
    Ready,
  };

  Status status;
};

}  // namespace oxm