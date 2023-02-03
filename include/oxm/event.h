#pragma once

#include <cstdint>
#include <cstdlib>

namespace oxm {

struct Event {
  using Id = size_t;
  using Mask = uint32_t;

  enum Type : uint32_t {
    None = 0,
    Read = 1 << 0,
    Write = 1 << 1,
    FileDescriptorError = 1 << 2,
    RemoteConnectionClosed = 1 << 3
  };

  void TriggerOn(Type type) {
    mask |= type;
  }

  int fd = -1;
  Mask mask = None;
};

inline bool HasError(Event::Mask mask) {
  return mask & oxm::Event::RemoteConnectionClosed || mask & oxm::Event::FileDescriptorError;
}

inline bool CanRead(Event::Mask mask) {
  return mask & oxm::Event::Read;
}

inline bool CanWrite(Event::Mask mask) {
  return mask & oxm::Event::Write;
}

}  // namespace oxm