#pragma once

#include <cstdint>

namespace oxm {

struct IAllocator {
  virtual ~IAllocator() = default;

  /// Allocates memory
  /// @param num_bytes number of bytes to allocate
  /// @return pointer to the allocated memory
  virtual void* Allocate(uint32_t num_bytes) = 0;

  /// Frees memory
  /// @param ptr pointer to memory memory to deallocate
  virtual void Deallocate(void* ptr) = 0;
};

}  // namespace oxm
