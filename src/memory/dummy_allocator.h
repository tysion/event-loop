#pragma once

#include "allocator_interface.h"

namespace oxm {

struct DummyAllocator final : IAllocator {
  DummyAllocator(uint32_t alignment) : alignment_{alignment} {
  }

  void* Allocate(uint32_t num_bytes) final;

  void Deallocate(void* ptr) final;

 private:
  const uint32_t alignment_ = 0;
};

}  // namespace oxm
