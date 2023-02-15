#pragma once

#include <memory>

#include "oxm/event_loop.h"

namespace oxm {

struct Task {
  virtual ~Task() = default;

  virtual void Execute(Event::Mask mask) = 0;
};

}  // namespace oxm