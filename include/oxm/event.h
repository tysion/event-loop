#pragma once

#include <cstdint>
#include <cstdlib>

namespace oxm {

struct Event {
  using Id = size_t;
  using Mask = uint32_t;

  enum class Type : uint32_t {
    Read = 1 << 0,
    Write = 1 << 1,
    FileDescriptorError = 1 << 2,
    RemoteConnectionClosed = 1 << 3
  };

  void TriggerOn(Type type) {
    mask |= static_cast<Mask>(type);
  }

  int fd = -1;
  Mask mask = 0;
};

inline bool Has(Event::Mask mask, Event::Type type) {
  return mask & static_cast<Event::Mask>(type);
}

inline void Set(Event::Mask& mask, Event::Type type) {
  mask |= static_cast<Event::Mask>(type);
}

inline bool HasError(Event::Mask mask) {
  return Has(mask, Event::Type::RemoteConnectionClosed) ||
         Has(mask, Event::Type::FileDescriptorError);
}

inline bool CanRead(Event::Mask mask) {
  return Has(mask, oxm::Event::Type::Read);
}

inline bool CanWrite(Event::Mask mask) {
  return Has(mask, oxm::Event::Type::Write);
}

}  // namespace oxm