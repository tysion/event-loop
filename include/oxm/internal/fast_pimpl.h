#pragma once

#include <cstdint>
#include <utility>

namespace omx {

template <typename T, uint64_t Size, uint64_t Alignment>
class FastPimpl {
 public:
  template <typename... Args>
  explicit FastPimpl(Args&&... args) {
    new (ptr()) T(std::forward<Args>(args)...);
  }

  FastPimpl& operator=(FastPimpl&& rhs) noexcept {
    *ptr() = std::move(*rhs);
    return *this;
  }

  T* operator->() noexcept {
    return ptr();
  }

  const T* operator->() const noexcept {
    return ptr();
  }

  T& operator*() noexcept {
    return *ptr();
  }

  const T& operator*() const noexcept {
    return *ptr();
  }

  ~FastPimpl() noexcept {
    validate<sizeof(T), alignof(T)>();
    ptr()->~T();
  }

  template <uint64_t ActualSize, uint64_t ActualAlignment>
  void validate() noexcept {
    static_assert(Size == ActualSize, "Size and sizeof(T) mismatch");
    static_assert(Alignment == ActualAlignment, "Alignment and alignof(T) mismatch");
  }

  T* ptr() noexcept {
    return reinterpret_cast<T*>(&m_data);
  }

  const T* ptr() const noexcept {
    return reinterpret_cast<const T*>(&m_data);
  }

 private:
  std::aligned_storage_t<Size, Alignment> m_data;
};

}  // namespace omx