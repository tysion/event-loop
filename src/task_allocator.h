#pragma once

#include "memory/buddy_allocator.h"
#include "memory/dummy_allocator.h"
#include "task.h"

namespace oxm {

struct TaskAllocator {
  static constexpr uint32_t kAlignment = 64;
  static constexpr uint32_t kTaskSize = sizeof(Task);

  TaskAllocator(size_t pool_size, IAllocator* parent = nullptr)
      : parent_{parent},
        dummy_{kAlignment},
        buddy_(parent ? parent : &dummy_, pool_size, kTaskSize)
  {}

  Task* AllocateTask() {
    return static_cast<Task*>(buddy_.Allocate(kTaskSize));
  }

  void DeallocateTask(Task* task) {
    buddy_.Deallocate(task);
  }

 private:
  IAllocator* parent_;
  DummyAllocator dummy_;
  BuddyAllocator buddy_;
};

}  // namespace oxm