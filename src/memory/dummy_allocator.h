#pragma once

#include "allocator_interface.h"

namespace oxm {

struct DummyAllocator final : IAllocator {
  void* Allocate(uint32_t num_bytes) final;

  void Deallocate(void* ptr) final;
};

}  // namespace oxm
