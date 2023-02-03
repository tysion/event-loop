#include <array>

#include "event_loop.h"

constexpr int kStdIn = 0;
constexpr int kStdOut = 1;
constexpr int kStdErr = 2;

int main() {
  oxm::EventLoop loop;

  std::array<char, 1024> buf = {};
  size_t n = 0;

  oxm::EventId write_ev1 = loop.RegisterEvent({kStdOut, oxm::EventType::ReadyToWriteTo});
  oxm::EventId write_ev2 = loop.RegisterEvent({kStdErr, oxm::EventType::ReadyToWriteTo});
  oxm::EventId read_ev = loop.RegisterEvent({kStdIn, oxm::EventType::ReadyToReadFrom});

  oxm::TaskPtr write_task1 = loop.CreateTask([&](oxm::Status status) {
    write(kStdOut, buf.data(), n);
  });

  oxm::TaskPtr write_task2 = loop.CreateTask([&](oxm::Status status) {
    write(kStdErr, buf.data(), n);
  });

  oxm::TaskPtr read_task = loop.CreateTask([&](oxm::Status status) {
    n = read(kStdIn, buf.data(), buf.size());
    loop.Schedule(write_ev1);
    loop.Schedule(write_ev2);
    loop.Schedule(read_ev);
  });

  loop.Bind(read_ev, read_task);
  loop.Bind(write_ev1, write_task1);
  loop.Bind(write_ev2, write_task2);
  loop.Schedule(read_ev);

  while (true) {
    loop.Poll();
  }

  return 0;
}
