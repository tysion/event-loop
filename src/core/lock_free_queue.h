#pragma once

#include <atomic>
#include <cassert>
#include <memory>

namespace oxm {

/// Multi-producer and multi-consumer lock-free array-based queue
/// https://www.1024cores.net/home/lock-free-algorithms/queues/bounded-mpmc-queue
template <typename T>
struct LockFreeQueue {
  explicit LockFreeQueue(size_t buffer_size)
      : buffer_(new Cell[buffer_size]), buffer_mask_(buffer_size - 1) {
    assert((buffer_size >= 2) && ((buffer_size & (buffer_size - 1)) == 0));
    for (size_t i = 0; i != buffer_size; i += 1) {
      buffer_[i].sequence_.store(i, std::memory_order_relaxed);
    }
    enqueue_pos_.store(0, std::memory_order_relaxed);
    dequeue_pos_.store(0, std::memory_order_relaxed);
  }

  ~LockFreeQueue() {
    delete[] buffer_;
  }

  bool TryPush(T data) {
    Cell* cell;
    size_t pos = enqueue_pos_.load(std::memory_order_relaxed);
    bool res = false;

    while (!res) {
      cell = &buffer_[pos & buffer_mask_];
      size_t seq = cell->sequence_.load(std::memory_order_acquire);
      intptr_t diff = intptr_t(seq) - intptr_t(pos);

      if (diff == 0) {
        res = enqueue_pos_.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed);
      } else if (diff < 0) {
        return false;
      } else {
        pos = enqueue_pos_.load(std::memory_order_relaxed);
      }
    }

    cell->data_ = std::move(data);
    cell->sequence_.store(pos + 1, std::memory_order_release);

    return true;
  }

  bool TryPop(T& data) {
    Cell* cell;
    size_t pos = dequeue_pos_.load(std::memory_order_relaxed);
    bool res = false;

    while (!res) {
      cell = &buffer_[pos & buffer_mask_];
      size_t seq = cell->sequence_.load(std::memory_order_acquire);
      intptr_t diff = intptr_t(seq) - intptr_t(pos + 1);

      if (diff == 0) {
        res = dequeue_pos_.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed);
      } else if (diff < 0) {
        return false;
      } else {
        pos = dequeue_pos_.load(std::memory_order_relaxed);
      }
    }

    data = std::move(cell->data_);
    cell->sequence_.store(pos + buffer_mask_ + 1, std::memory_order_release);

    return true;
  }

 private:
  struct Cell {
    std::atomic<size_t> sequence_;
    T data_;
  };

  alignas(64) Cell* const buffer_;
  alignas(64) const size_t buffer_mask_;
  alignas(64) std::atomic<size_t> enqueue_pos_{};
  alignas(64) std::atomic<size_t> dequeue_pos_{};
};

}  // namespace oxm