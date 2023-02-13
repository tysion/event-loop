#include <array>
#include <cstring>

#include "oxm/event_loop.h"

constexpr int kStdIn = 0;
constexpr int kStdOut = 1;
constexpr int kStdErr = 2;

int main() {
  auto loop = std::make_shared<oxm::EventLoop>();

  oxm::Event print_input_event;
  print_input_event.fd = kStdOut;
  print_input_event.mask.Set(oxm::Event::Type::Write);

  oxm::Event print_error_event;
  print_error_event.fd = kStdErr;
  print_error_event.mask.Set(oxm::Event::Type::Write);

  oxm::Event read_input_event;
  read_input_event.fd = kStdIn;
  read_input_event.mask.Set(oxm::Event::Type::Read);
  read_input_event.mask.Set(oxm::Event::Type::Write);

  const oxm::Event::Id print_input = loop->RegisterEvent(print_input_event);
  const oxm::Event::Id print_error = loop->RegisterEvent(print_error_event);
  const oxm::Event::Id read_input = loop->RegisterEvent(read_input_event);

  std::array<char, 1024> buf = {};
  size_t n = 0;

  oxm::TaskPtr print_input_task = loop->CreateTask([&](oxm::Event::Mask mask) {
    if (mask.HasError()) {
      loop->Schedule(print_error);
    }

    if (mask.CanWrite()) {
      auto _ = write(kStdOut, buf.data(), n);
    }

    loop->Unshedule(print_input, false);
  });

  oxm::TaskPtr print_error_task = loop->CreateTask([&](oxm::Event::Mask mask) {
    if (mask.HasError()) {
      throw std::runtime_error("internal error");
    }

    if (mask.CanWrite()) {
      auto _ = write(kStdErr, "error happened", 15);
    }

    loop->Unshedule(print_error, false);
  });

  oxm::TaskPtr read_input_task = loop->CreateTask([&](oxm::Event::Mask mask) {
    if (mask.HasError()) {
      loop->Schedule(print_error);
    }

    if (mask.CanRead()) {
      n = read(kStdIn, buf.data(), buf.size());

      if (mask.CanWrite()) {
        auto _ = write(kStdIn, "> ", 2);
      }

      loop->Schedule(print_input);
    }
  });

  loop->Bind(read_input, read_input_task);
  loop->Bind(print_input, print_input_task);
  loop->Bind(print_error, print_error_task);

  loop->Schedule(read_input);

  while (true) {
    loop->Poll();
  }

  return 0;
}
