#pragma once

#include <memory>

#include "event_loop.h"

namespace oxm {

struct Task {
  explicit Task(std::function<void(Status)>&& callback) : callback_(std::move(callback)) {
  }

  void Execute(Status status) {
    callback_(status);
  }

 private:
  Callback callback_;
};

}  // namespace oxm