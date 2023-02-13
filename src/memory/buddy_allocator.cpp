#include "buddy_allocator.h"

#include <cassert>
#include <tuple>

namespace oxm {

namespace {

/// Rounds up to the nearest power of two
/// @return the smallest power of two value greater than x
uint32_t RoundUp(uint32_t x) {
  // __builtin_clz counts number of leading zeroes in the binary representation of a number.
  // example:
  // x = 30 (00000000 00000000 00000000 00011110)
  // __builtin_clz(30) will return 27
  return 1 << (32 - __builtin_clz(x - 1));
}

/// Calculates number of blocks in binary tree
uint32_t CalculateNumberOfBlocks(uint32_t num_levels) {
  return (1 << num_levels) - 1;
}

/// Calculates number of blocks in binary tree level
uint32_t CalculateNumberOfBlocksOnLevel(uint32_t level_index) {
  return 1 << level_index;
}

/// Checks if number is power of two
/// @return true if x if power of 2 and false otherwise
bool IsPowerOfTwo(uint32_t x) {
  // __builtin_popcount counts the number of one’s in the binary representation of a number.
  // example:
  // x = 30 (00000000 00000000 00000000 00011110)
  // __builtin_popcount(30) will return 4
  return __builtin_popcount(x) == 1;
}

/// Calculates depth of the binary tree
/// @param blob_size - total number of bytes
/// @param min_block_size - minimal memory block size
/// @note blob_size and min_block_size must be power of 2
/// @note blob_size must greater or equal to min_block_size
uint32_t CalculateNumberOfLevels(uint32_t blob_size, uint32_t min_block_size) {
  assert(IsPowerOfTwo(blob_size));
  assert(IsPowerOfTwo(min_block_size));
  assert(blob_size >= min_block_size);
  // __builtin_ctz counts the number of trailing zeroes in the binary representation of a number.
  // example:
  // x = 30 (00000000 00000000 00000000 00011110)
  // __builtin_ctz(30) will return 1
  return __builtin_ctz(blob_size) - __builtin_ctz(min_block_size) + 1;
}

}  // namespace

BuddyAllocator::BuddyAllocator(IAllocator* parent, uint32_t blob_size, uint32_t min_block_size)
    : parent_{parent},
      data_size_{RoundUp(blob_size)},
      min_block_size_{RoundUp(min_block_size)},
      level_count_{CalculateNumberOfLevels(data_size_, min_block_size_)},
      block_count_{CalculateNumberOfBlocks(level_count_)} {
  assert(parent_);
}

BuddyAllocator::~BuddyAllocator() {
  if (data_) {
    parent_->Deallocate(data_);
  }
}

void BuddyAllocator::Init() {
  if (data_ == nullptr) {
    void* ptr = parent_->Allocate(block_count_ + data_size_);
    if (ptr == nullptr) {
      throw std::bad_alloc();
    }

    data_ = static_cast<uint8_t*>(ptr);
    statuses_ = reinterpret_cast<BlockStatus*>(data_ + data_size_);
    std::fill_n(statuses_, block_count_, BlockStatus::Free);
  }
}

uint32_t BuddyAllocator::GetParentIndex(uint32_t index) const {
  assert(index > 0);
  assert(index < block_count_);
  return (index - 1) / 2;
}

uint32_t BuddyAllocator::GetBuddyIndex(uint32_t index) const {
  assert(index > 0);
  assert(index < block_count_);
  return index % 2 == 0 ? index - 1 : index + 1;
}

std::optional<uint32_t> BuddyAllocator::FindFreeBlock(uint32_t beg, uint32_t end) const {
  // TODO: optimize linear scan with avx2 instructions
  for (uint32_t index = beg; index < end; ++index) {
    if (statuses_[index] == BlockStatus::Free) {
      return index;
    }
  }

  return std::nullopt;
}

void BuddyAllocator::TraverseAndMark(uint32_t block_index, uint32_t level_index,
                                     BlockStatus status) {
  uint32_t num_blocks = 1;

  for (level_index += 1; level_index < level_count_; ++level_index) {
    // go to the left child index
    block_index = block_index * 2 + 1;
    // num children increase in 2 times with each level
    num_blocks *= 2;
    // set all children status to given
    std::fill_n(statuses_ + block_index, num_blocks, status);
  }
}

void* BuddyAllocator::Allocate(uint32_t num_bytes) {
  if (num_bytes > data_size_) {
    return nullptr;
  }

  // does nothing if already allocated
  Init();

  // can't allocate less than min_block_size_ number of bytes
  // can't allocate non-power of two number of bytes
  num_bytes = std::max(RoundUp(num_bytes), min_block_size_);

  // level index of blocks with size of num_bytes is equal to number of blocks in tree
  // where min block is size of num_bytes minus one because last index == size - 1
  const auto level_index = CalculateNumberOfLevels(data_size_, num_bytes) - 1;
  // first index == number of nodes in binary tree with level_index depth
  const auto level_beg = CalculateNumberOfBlocks(level_index);
  const auto level_end = level_beg + CalculateNumberOfBlocksOnLevel(level_index);

  const auto index_opt = FindFreeBlock(level_beg, level_end);
  if (!index_opt.has_value()) {
    return nullptr;
  }

  const auto block_index = *index_opt;

  // traverse subtree starting from the index node and mark all child nodes as used
  TraverseAndMark(block_index, level_index, BlockStatus::Used);

  // go up to the root marking all parent nodes as split
  for (auto index = block_index; index > 0; index = GetParentIndex(index)) {
    // early stopping
    if (statuses_[index] == BlockStatus::Split) {
      break;
    }
    statuses_[index] = BlockStatus::Split;
  }
  statuses_[0] = BlockStatus::Split;
  statuses_[block_index] = BlockStatus::Allocated;

  return data_ + (block_index - level_beg) * num_bytes;

  return nullptr;
}

void BuddyAllocator::Deallocate(void* ptr) {
  const auto offset = static_cast<uint8_t*>(ptr) - data_;
  const auto blocks_offset = offset / min_block_size_;
  const auto level_beg = CalculateNumberOfBlocks(level_count_ - 1);

  // find allocated block
  auto block_index = level_beg + blocks_offset;
  auto level_index = level_count_ - 1;
  while (statuses_[block_index] != BlockStatus::Allocated) {
    block_index = GetParentIndex(block_index);
    --level_index;
  }

  // go up to the root coalescing with buddy if it is also free
  for (auto index = block_index; index > 0; index = GetParentIndex(index)) {
    statuses_[index] = BlockStatus::Free;

    const auto buddy_index = GetBuddyIndex(index);
    if (statuses_[buddy_index] != BlockStatus::Free) {
      break;
    }
  }

  // traverse subtree starting from the index node and mark all child nodes as free
  TraverseAndMark(block_index, level_index, BlockStatus::Free);
}

}  // namespace oxm
