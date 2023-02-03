#pragma once

#include <memory>

#include "event_loop.h"

namespace oxm {

struct Task {
  explicit Task(Callback&& callback) : callback_(std::move(callback)) {
  }

  void Execute(Event::Mask mask) {
    callback_(mask);
  }

 private:
  Callback callback_;
};

}  // namespace oxm