#pragma once

#include <atomic>
#include <cassert>
#include <memory>

namespace oxm {

/// Multi-producer and multi-consumer lock-free array-based queue
/// https://www.1024cores.net/home/lock-free-algorithms/queues/bounded-mpmc-queue
template <typename T>
struct LockFreeQueue {
  explicit LockFreeQueue(size_t buffer_size);

  ~LockFreeQueue() {
    delete[] buffer_;
  }

  bool TryPush(T data);

  bool TryPop(T& data);

 private:
  struct Cell {
    std::atomic<size_t> sequence_;
    T data_;
  };

  enum class Operation {
    Enqueue,
    Dequeue
  };

  template <Operation Op>
  std::pair<Cell*, size_t> FindCell() {
    static_assert(Op == Operation::Enqueue || Op == Operation::Dequeue, "Invalid operation");

    Cell* cell;
    std::atomic<size_t>& atomic_pos = Op == Operation::Enqueue ? enqueue_pos_ : dequeue_pos_;
    size_t pos = atomic_pos.load(std::memory_order_relaxed);

    bool res = false;
    while (!res) {
      cell = &buffer_[pos & buffer_mask_];
      size_t seq = cell->sequence_.load(std::memory_order_acquire);

      intptr_t diff;
      if constexpr (Op == Operation::Enqueue) {
        diff = intptr_t(seq) - intptr_t(pos);
      } else {
        diff = intptr_t(seq) - intptr_t(pos + 1);
      }

      if (diff == 0) {
        res = atomic_pos.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed);
      } else if (diff < 0) {
        return {nullptr, 0};
      } else {
        pos = atomic_pos.load(std::memory_order_relaxed);
      }
    }

    return {cell, pos};
  }

  alignas(64) Cell* const buffer_;
  alignas(64) const size_t buffer_mask_;
  alignas(64) std::atomic<size_t> enqueue_pos_{};
  alignas(64) std::atomic<size_t> dequeue_pos_{};
};

template <typename T>
LockFreeQueue<T>::LockFreeQueue(size_t buffer_size)
    : buffer_(new Cell[buffer_size]), buffer_mask_(buffer_size - 1) {
  assert((buffer_size >= 2) && ((buffer_size & (buffer_size - 1)) == 0));
  for (size_t i = 0; i != buffer_size; i += 1) {
    buffer_[i].sequence_.store(i, std::memory_order_relaxed);
  }
  enqueue_pos_.store(0, std::memory_order_relaxed);
  dequeue_pos_.store(0, std::memory_order_relaxed);
}

template <typename T>
bool LockFreeQueue<T>::TryPush(T data) {
  auto [cell, pos] = FindCell<Operation::Enqueue>();
  if (!cell) {
    return false;
  }

  cell->data_ = std::move(data);
  cell->sequence_.store(pos + 1, std::memory_order_release);

  return true;
}

template <typename T>
bool LockFreeQueue<T>::TryPop(T& data) {
  auto [cell, pos] = FindCell<Operation::Dequeue>();
  if (!cell) {
    return false;
  }

  data = std::move(cell->data_);
  cell->sequence_.store(pos + buffer_mask_ + 1, std::memory_order_release);

  return true;
}

}  // namespace oxm