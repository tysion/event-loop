#pragma once

#include <memory>

#include "oxm/event.h"
#include "oxm/task.h"

namespace oxm {

struct Executor;

using ExecutorPtr = std::shared_ptr<Executor>;
using Predicate = bool (*)(void* user_data);
using FnPushTask = void (*)(Task* task, oxm::Event::Mask mask, void* ctx);
using FnWait = void (*)(Predicate predicate, void* user_data, void* ctx);
using FnTerminate = void (*)(void* ctx);

struct Executor {
  void PushTask(Task* task, oxm::Event::Mask mask) {
    push_task_fn(task, mask, ctx);
  }

  void Wait(Predicate predicate, void* user_data) {
    wait_fn(predicate, user_data, ctx);
  }

  ~Executor() {
    terminate_fn(ctx);
  }

  void* ctx = nullptr;
  FnPushTask push_task_fn{};
  FnWait wait_fn{};
  FnTerminate terminate_fn{};
};
}  // namespace oxm
