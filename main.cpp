#include <array>
#include <cstring>

#include "event_loop.h"

constexpr int kStdIn = 0;
constexpr int kStdOut = 1;
constexpr int kStdErr = 2;

int main() {
  auto loop = std::make_shared<oxm::EventLoop>();

  oxm::Event print_input_event;
  print_input_event.fd = kStdOut;
  print_input_event.TriggerOn(oxm::Event::Type::Write);

  oxm::Event print_error_event;
  print_error_event.fd = kStdErr;
  print_error_event.TriggerOn(oxm::Event::Type::Write);

  oxm::Event read_input_event;
  read_input_event.fd = kStdIn;
  read_input_event.TriggerOn(oxm::Event::Type::Read);

  const oxm::Event::Id print_input = loop->RegisterEvent(print_input_event);
  const oxm::Event::Id print_error = loop->RegisterEvent(print_error_event);
  const oxm::Event::Id read_input = loop->RegisterEvent(read_input_event);

  std::array<char, 1024> buf = {};
  size_t n = 0;

  oxm::TaskPtr print_input_task = loop->CreateTask([&](oxm::Event::Mask mask) {
    if (oxm::HasError(mask)) {
      loop->Schedule(print_error);
    }

    if (oxm::CanWrite(mask)) {
      write(kStdOut, buf.data(), n);
    }
  });

  oxm::TaskPtr print_error_task = loop->CreateTask([&](oxm::Event::Mask mask) {
    if (oxm::HasError(mask)) {
      throw std::runtime_error("internal error");
    }

    if (oxm::CanWrite(mask)) {
      write(kStdErr, "error happened", 15);
    }
  });

  oxm::TaskPtr read_input_task = loop->CreateTask([&](oxm::Event::Mask mask) {
    if (oxm::HasError(mask)) {
      loop->Schedule(print_error);
    }

    if (oxm::CanRead(mask)) {
      n = read(kStdIn, buf.data(), buf.size());

      loop->Schedule(print_input);
    }

    loop->Schedule(read_input);
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
