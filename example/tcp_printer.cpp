#include <arpa/inet.h>
#include <fcntl.h>
#include <oxm/event_loop.h>

#include <utility>

using EventLoopPtr = std::shared_ptr<oxm::EventLoop>;

struct TcpAcceptor {
  explicit TcpAcceptor(EventLoopPtr loop, int listen_socket)
      : loop_{std::move(loop)}, listen_socket_{listen_socket} {
    oxm::Event accept_event;
    accept_event.TriggerOn(oxm::Event::Read);
    accept_event.fd = listen_socket_;

    accept_event_id_ = loop_->RegisterEvent(accept_event);
    oxm::TaskPtr on_accept = loop_->CreateTask([this](oxm::Event::Mask mask) {
      if (oxm::HasError(mask)) {
        HandleError(mask, accept_event_id_);
        return;
      }

      const int connection_socket = Accept();
      HandleConnection(connection_socket);
    });

    loop_->Bind(accept_event_id_, on_accept);
  }

  void AcceptAsync() {
    loop_->Schedule(accept_event_id_);
  }

 private:
  void HandleConnection(int connection_socket) {
    oxm::Event connection_event;
    connection_event.fd = connection_socket;
    connection_event.TriggerOn(oxm::Event::Read);

    oxm::Event::Id connection_event_id = loop_->RegisterEvent(connection_event);
    oxm::TaskPtr on_connect =
        loop_->CreateTask([this, connection_event_id, connection_socket](oxm::Event::Mask mask) {
          if (oxm::HasError(mask)) {
            HandleError(mask, connection_event_id);
            return;
          }

          if (oxm::CanRead(mask)) {
            char buffer[4096];
            auto len = read(connection_socket, &buffer, 4096);
            if (len > 0) {
              write(1, buffer, len);
            }
          }
        });

    loop_->Bind(connection_event_id, on_connect);
    loop_->Schedule(connection_event_id);
  }

  void HandleError(oxm::Event::Mask mask, oxm::Event::Id id) {
    if (mask & oxm::Event::RemoteConnectionClosed) {
      printf("Error: Remote connection was closed");
    } else if (mask & oxm::Event::FileDescriptorError) {
      printf("Error: EPOLL: File descriptor error");
    } else {
      printf("Error: Unknown error");
    }

    loop_->Unshedule(id, true);
  }

  int Accept() const {
    sockaddr_in address = {};
    socklen_t address_size = sizeof(address);

    int connection_socket = accept(listen_socket_, (sockaddr*)&address, &address_size);
    if (connection_socket == -1) {
      throw std::runtime_error("Accept failed");
    }

    if (fcntl(connection_socket, F_SETFL, O_NONBLOCK) == -1) {
      throw std::runtime_error("Make non blocking failed");
    }

    return connection_socket;
  }

  const int listen_socket_;
  oxm::Event::Id accept_event_id_;
  EventLoopPtr loop_;
};

TcpAcceptor Listen(EventLoopPtr loop, const char* address, int port) {
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

  return TcpAcceptor(std::move(loop), listen_socket);
}

int main() {
  auto loop = std::make_shared<oxm::EventLoop>();

  auto acceptor = Listen(loop, "0.0.0.0", 9000);

  acceptor.AcceptAsync();

  while (true) {
    loop->Poll();
  }
}