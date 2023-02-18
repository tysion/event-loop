#include <arpa/inet.h>
#include <fcntl.h>
#include <oxm/event_loop.h>
#include <unistd.h>

#include <iostream>
#include <utility>

struct TcpAcceptor;
struct TcpConnection;

using EventLoopPtr = std::shared_ptr<oxm::EventLoop>;
using TcpAcceptorPtr = std::shared_ptr<TcpAcceptor>;
using TcpConnectionPtr = std::shared_ptr<TcpConnection>;

void PrintErrorMessage(oxm::Event::Mask mask) {
  if (mask.Has(oxm::Event::Type::RemoteConnectionClosed)) {
    printf("Error: Remote connection was closed");
  } else if (mask.Has(oxm::Event::Type::FileDescriptorError)) {
    printf("Error: EPOLL: File descriptor error");
  } else {
    printf("Error: Unknown error");
  }
}

struct TcpConnection : std::enable_shared_from_this<TcpConnection> {
  TcpConnection(EventLoopPtr loop, int socket) : loop_{std::move(loop)}, socket_{socket} {
    oxm::Event event;
    event.fd = socket_;
    event.mask.Set(oxm::Event::Type::Read);
    event.mask.Set(oxm::Event::Type::Write);
    event_id_ = loop_->RegisterEvent(event);

    CleanBuffer();

    printf("%d joined server\n", socket_);
  }

  ~TcpConnection() {
    close(socket_);
    printf("%d left server\n", socket_);
  }

  void HandleAsync() {
    oxm::Task* on_connect = loop_->CreateTask([self = shared_from_this()](oxm::Event::Mask mask) {
      if (mask.HasError()) {
        self->OnError(mask);
        return;
      }

      if (mask.CanRead()) {
        self->OnRead();
      }

      if (mask.CanWrite()) {
        self->OnWrite();
      }
    });

    loop_->Bind(event_id_, on_connect);
    loop_->Schedule(event_id_);
  }

 private:
  void OnError(oxm::Event::Mask mask) {
    PrintErrorMessage(mask);
    loop_->Unshedule(event_id_, true);
  }

  void OnRead() {
    if (hasRead) {
      return;
    }

    auto len = read(socket_, buffer.data(), buffer.size());
    if (len > 0) {
      hasRead = true;
      printf("[%d]: %s", socket_, buffer.data());
    }
  }

  void OnWrite() {
    if (!hasRead) {
      return;
    }

    auto len = write(socket_, buffer.data(), buffer.size());
    if (len < 0) {
      throw std::runtime_error("OnWrite");
    }

    hasRead = false;
    CleanBuffer();
  }

  void CleanBuffer() {
    buffer.fill('\0');
  }

  const int socket_;
  oxm::Event::Id event_id_;
  EventLoopPtr loop_;
  bool hasRead = false;

  std::array<char, 1024> buffer = {};
};

struct TcpAcceptor : std::enable_shared_from_this<TcpAcceptor> {
  TcpAcceptor(EventLoopPtr loop, int listen_socket)
      : loop_{std::move(loop)}, socket_{listen_socket} {
    oxm::Event event;
    event.fd = socket_;
    event.mask.Set(oxm::Event::Type::Read);
    event_id_ = loop_->RegisterEvent(event);
  }

  void AcceptAsync() {
    oxm::Task* on_accept = loop_->CreateTask([self = shared_from_this()](oxm::Event::Mask mask) {
      if (mask.HasError()) {
        self->OnError(mask);
        return;
      }

      self->OnConnect();
    });

    loop_->Bind(event_id_, on_accept);
    loop_->Schedule(event_id_);
  }

 private:
  void OnError(oxm::Event::Mask mask) {
    PrintErrorMessage(mask);
    loop_->Unshedule(event_id_, true);
  }

  void OnConnect() const {
    sockaddr_in address = {};
    socklen_t address_size = sizeof(address);

    int connection_socket = accept(socket_, (sockaddr*)&address, &address_size);
    if (connection_socket == -1) {
      throw std::runtime_error("Accept failed");
    }

    if (fcntl(connection_socket, F_SETFL, O_NONBLOCK) == -1) {
      throw std::runtime_error("Make non blocking failed");
    }

    std::make_shared<TcpConnection>(loop_, connection_socket)->HandleAsync();
  }

  const int socket_;
  oxm::Event::Id event_id_;
  EventLoopPtr loop_;
};

TcpAcceptorPtr Listen(EventLoopPtr loop, const char* address, int port) {
  int listen_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (listen_socket < 0) {
    throw std::runtime_error("Socket cannot be created!");
  }

  sockaddr_in listen_address = {.sin_family = AF_INET, .sin_port = htons(port)};

  inet_pton(AF_INET, address, &listen_address.sin_addr);

  if (bind(listen_socket, (sockaddr*)&listen_address, sizeof(listen_address)) < 0) {
    throw std::runtime_error("Socket cannot be created!");
  }

  if (listen(listen_socket, SOMAXCONN) < 0) {
    throw std::runtime_error("Socket cannot be switched to listen mode!");
  }

  return std::make_shared<TcpAcceptor>(std::move(loop), listen_socket);
}

int main() {
  auto loop = std::make_shared<oxm::EventLoop>();

  Listen(loop, "0.0.0.0", 9000)->AcceptAsync();

  while (true) {
    loop->Poll();
  }
}