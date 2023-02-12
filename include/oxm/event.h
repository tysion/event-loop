#pragma once

#include <cstdint>
#include <cstdlib>

namespace oxm {

struct Event {
  using Id = size_t;

  enum class Type : uint32_t {
    Read = 1 << 0,
    Write = 1 << 1,
    FileDescriptorError = 1 << 2,
    RemoteConnectionClosed = 1 << 3
  };

  struct Mask {
    void Set(Type type) {
      bits |= static_cast<typeof(Mask::bits)>(type);
    }

    bool Has(Type type) {
      return bits & static_cast<typeof(Mask::bits)>(type);
    }

    bool HasError() {
      return Has(Type::FileDescriptorError) ||
             Has(Type::RemoteConnectionClosed);
    }

    bool CanRead() {
      return Has(Type::Read);
    }

    bool CanWrite() {
      return Has(Type::Write);
    }

    bool IsValid() {
      return true;
    }

    uint32_t bits = 0;
  };

  int fd = -1;
  Mask mask = {};
};

}  // namespace oxm