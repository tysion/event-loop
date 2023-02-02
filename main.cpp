#include <array>

#include "event_loop.h"

constexpr int kStdIn = 0;
constexpr int kStdOut = 1;

int main() {
  oxm::EventLoop loop;
  std::array<char, 1024> buf = {};

  loop.ExecuteWhen(kStdIn, oxm::EventType::ReadyToReadFrom,
    [&buf, inp_fd = kStdIn, out_fd = kStdOut](oxm::EventLoop* loop) {
      size_t n = read(inp_fd, buf.data(), buf.size());
      write(out_fd, buf.data(), n);
    });

  while (true) {
    loop.Poll();
  }

  return 0;
}
