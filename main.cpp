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

    // выполни колбэк Process когда fd будет готов для чтения
    epoll->ExecuteWhenReady(fd, oxm::EventType::ReadyToReadFrom, Process);
  };

  // выполни колбэк on_ready_to_write когда fd (stdout) будет готов для записи
  epoll->ExecuteWhenReady(kStdOut, oxm::EventType::ReadyToWriteTo, on_ready_to_write);
}

int main() {
  oxm::Epoll epoll;

  auto on_ready_to_read = [](int fd, oxm::Epoll* epoll) { Process(fd, epoll); };

  // выполни колбэк on_ready_to_read when когда fd (stdin) будет готов для чтения
  epoll.ExecuteWhenReady(kStdIn, oxm::EventType::ReadyToReadFrom, on_ready_to_read);

  while (true) {
    // опроси все отслеживаемые дескрипторы и выполни соотвествующие колбэки
    epoll.Poll();
  }

  return 0;
}
