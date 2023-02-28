#pragma once

#include "memory/buddy_allocator.h"
#include "oxm/task.h"

namespace oxm {

struct TaskAllocator {
  static constexpr uint32_t kMinTaskSize = 16;
  static constexpr uint32_t kAlignment = 64;

  explicit TaskAllocator(size_t pool_size)
      : allocator_(&base_, pool_size, kMinTaskSize) {
  }

  Task* Allocate(size_t task_size) {
    return static_cast<Task*>(allocator_.Allocate(task_size));
  }

  void Deallocate(Task* task) {
    allocator_.Deallocate(task);
  }

 private:
  AlignedAllocator<kAlignment> base_;
  AlignedBuddyAllocator<kAlignment> allocator_;
};

}  // namespace oxm