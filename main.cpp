#include <unistd.h>

#include <array>

#include "epoll.h"

constexpr int kStdIn = 0;
constexpr int kStdOut = 1;

void Process(int fd, oxm::Epoll* epoll) {
  std::array<char, 1024> buf = {};
  size_t n = read(fd, buf.data(), buf.size());

  auto on_ready_to_write = [buf, n](int fd, oxm::Epoll* epoll) {
    write(fd, buf.data(), n);

    epoll->ExecuteWhen(fd, oxm::EventType::ReadyToReadFrom, Process);
  };

  epoll->ExecuteWhen(kStdOut, oxm::EventType::ReadyToWriteTo, on_ready_to_write);
}

int main() {
  oxm::Epoll epoll;

  auto on_ready_to_read = [](int fd, oxm::Epoll* epoll) { Process(fd, epoll); };

  epoll.ExecuteWhen(kStdIn, oxm::EventType::ReadyToReadFrom, on_ready_to_read);

  while (true) {
    epoll.Poll();
  }

  return 0;
}
