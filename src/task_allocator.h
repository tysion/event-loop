#pragma once

#include "memory/buddy_allocator.h"
#include "memory/dummy_allocator.h"
#include "oxm/task.h"

namespace oxm {

struct TaskAllocator {
  static constexpr uint32_t kAlignment = 64;
  static constexpr uint32_t kMinTaskSize = 32;

  TaskAllocator(size_t pool_size, IAllocator* parent = nullptr)
      : parent_{parent},
        dummy_{kAlignment},
        buddy_(parent ? parent : &dummy_, pool_size, kMinTaskSize) {
  }

  Task* Allocate(size_t task_size) {
    return static_cast<Task*>(buddy_.Allocate(task_size));
  }

  void Deallocate(Task* task) {
    buddy_.Deallocate(task);
  }

 private:
  IAllocator* parent_;
  DummyAllocator dummy_;
  BuddyAllocator buddy_;
};

}  // namespace oxm