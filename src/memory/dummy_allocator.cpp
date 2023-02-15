#include "dummy_allocator.h"

#include <cstdlib>

namespace oxm {

void* DummyAllocator::Allocate(uint32_t num_bytes) {
  return malloc(num_bytes);
}

void DummyAllocator::Deallocate(void* ptr) {
  return std::free(ptr);
}

}  // namespace oxm
