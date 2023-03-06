#pragma once

#include <memory>

#include "oxm/event.h"

namespace oxm {

struct Task {
  virtual ~Task() = default;

  virtual void Execute(Event::Mask mask) = 0;
};

}  // namespace oxm