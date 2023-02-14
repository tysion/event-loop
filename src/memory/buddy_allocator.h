#pragma once

#include "dummy_allocator.h"

#include <cstdint>
#include <vector>
#include <optional>

namespace oxm {

struct BuddyAllocator final : IAllocator {
  enum class BlockStatus : uint8_t {
    Free = 0, /// block is free
    Allocated = 1, /// block is allocated
    Used = 2, /// block is part of the allocated block
    Split = 3 /// at least one part of this block was allocated
  };

  BuddyAllocator(IAllocator* parent, uint32_t blob_size, uint32_t min_block_size);

  ~BuddyAllocator() final;

  void* Allocate(uint32_t num_bytes) final;

  void Deallocate(void* ptr) final;

  uint32_t GetBlockCount() const {
    return block_count_;
  }

  uint32_t GetLevelCount() const {
    return level_count_;
  }

  bool Init();

 private:
  uint32_t GetParentIndex(uint32_t index) const;

  uint32_t GetBuddyIndex(uint32_t index) const;

  std::optional<uint32_t> FindFreeBlock(uint32_t beg, uint32_t n) const;

  void TraverseAndMark(uint32_t block_index, uint32_t level_index, BlockStatus status);

  IAllocator* parent_ = nullptr;
  const uint32_t data_size_;
  const uint32_t min_block_size_;
  const uint32_t level_count_;
  const uint32_t block_count_;
  uint8_t* data_ = nullptr;
  BlockStatus* statuses_ = nullptr;
};

}