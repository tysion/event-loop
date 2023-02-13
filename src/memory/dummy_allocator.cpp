#include "dummy_allocator.h"

#include <cstdlib>

namespace oxm {

void* DummyAllocator::Allocate(uint32_t num_bytes) {
  return std::aligned_alloc(alignment_, num_bytes);
}

void DummyAllocator::Deallocate(void* ptr) {
  return std::free(ptr);
}

}
