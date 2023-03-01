#pragma once

#include <cstdint>

namespace oxm {

template <typename TAllocator>
struct BaseAllocator {
  /// Allocates memory
  /// @param num_bytes number of bytes to allocate
  /// @return pointer to the allocated memory
  void* Allocate(uint32_t num_bytes) {
    return Derived()->AllocateImpl(num_bytes);
  }

  /// Frees memory
  /// @param ptr pointer to memory memory to deallocate
  void Deallocate(void* ptr) {
    Derived()->DeallocateImpl(ptr);
  }

 private:
  TAllocator* Derived() {
    return static_cast<TAllocator*>(this);
  }
};

}  // namespace oxm
