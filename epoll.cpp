#include "epoll.h"

namespace oxm {

struct Task {
  Task(int fd, Callback cb, Epoll* epoll, void* user_data)
      : fd_{fd}, cb_{cb}, epoll_{epoll}, user_data_{user_data} {
  }

  void Execute() {
    cb_(fd_, epoll_, user_data_);
  }

  int GetFileDescriptor() const {
    return fd_;
  }

 private:
  int fd_;
  Callback cb_;
  Epoll* epoll_;
  void* user_data_;
};

Epoll::Epoll(int approximate_events_count) {
  epfd_ = epoll_create1(EPOLL_CLOEXEC);
  if (epfd_ < 0) {
    throw std::runtime_error("failed to epoll_create1");
  }

  ready_events_.reserve(approximate_events_count);
}

void Epoll::ExecuteWhenReady(int fd, EventType event_type, Callback cb, void* user_data) {
  auto* task = new Task(fd, cb, this, user_data);

  epoll_event ev{};
  ev.data.ptr = task;
  ev.events = ToEpollEvent(event_type);

  TrackDescriptor(fd, &ev);
}

void Epoll::Poll() {
  ready_events_.resize(events_count_);

  int num_ready_events = epoll_wait(epfd_, ready_events_.data(), events_count_, -1);
  if (num_ready_events < 0) {
    throw std::runtime_error("failed to epoll_wait");
  }

  for (int i = 0; i < num_ready_events; ++i) {
    auto* task = static_cast<Task*>(ready_events_[i].data.ptr);

    UntrackDescriptor(task->GetFileDescriptor());

    task->Execute();

    delete task;
  }
}

int Epoll::ToEpollEvent(EventType event_type) {
  switch (event_type) {
    case EventType::ReadyToReadFrom:
      return EPOLLIN | EPOLLHUP | EPOLLERR;
    case EventType::ReadyToWriteTo:
      return EPOLLOUT | EPOLLHUP | EPOLLERR;
  }
}

void Epoll::TrackDescriptor(int fd, epoll_event* ev) {
  int err = epoll_ctl(epfd_, EPOLL_CTL_ADD, fd, ev);
  if (err < 0) {
    throw std::runtime_error("failed to epoll_ctl");
  }
  ++events_count_;
}

void Epoll::UntrackDescriptor(int fd) {
  int err = epoll_ctl(epfd_, EPOLL_CTL_DEL, fd, nullptr);
  if (err < 0) {
    throw std::runtime_error("failed to epoll_ctl");
  }
  --events_count_;
}

}  // namespace oxm
