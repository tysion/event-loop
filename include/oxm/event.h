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
    RemoteConnectionClosed = 1 << 3,

    ReadWrite = Read | Write
  };

  struct Mask {
    void Set(Type type) {
      bits |= static_cast<decltype(Mask::bits)>(type);
    }

    bool Has(Type type) {
      return bits & static_cast<decltype(Mask::bits)>(type);
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

inline Event MakeEvent(int fd, std::initializer_list<Event::Type> event_types) {
  Event event;
  event.fd = fd;
  for (const auto event_type: event_types) {
    event.mask.Set(event_type);
  }
  return event;
}

}  // namespace oxm