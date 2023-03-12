#pragma once

#include <utility>

#include "task_executor.h"

namespace oxm {

struct AsyncMutex {
  explicit AsyncMutex(ExecutorPtr executor) : executor_{std::move(executor)} {
  }

  void lock() {
    executor_->Wait([this]() -> bool {
      return !lock_.load(std::memory_order_relaxed) &&
             !lock_.exchange(true, std::memory_order_acquire);
    });
  }

  void unlock() {
    lock_.store(false, std::memory_order_release);
  }

 private:
  ExecutorPtr executor_;
  std::atomic<bool> lock_ = false;
};

}  // namespace oxm
