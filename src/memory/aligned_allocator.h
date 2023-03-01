#pragma once

#include <cstdlib>

#include "base_allocator.h"

namespace oxm {

template <uint32_t TAlignement>
struct AlignedAllocator final : BaseAllocator<AlignedAllocator<TAlignement>> {
  void* AllocateImpl(uint32_t num_bytes) {
    return std::aligned_alloc(TAlignement, num_bytes);
  }

  void DeallocateImpl(void* ptr) {
    std::free(ptr);
  }
};

}  // namespace oxm
