#include <iostream>
#include <oxm/event_loop.h>

#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>

const int MAX_EVENTS = 10;

int createSocket() {
  int listen_sock = socket(AF_INET, SOCK_STREAM, 0);
  if (listen_sock < 0) {
    perror("[ERROR:1] Socket cannot be created!");
    exit(EXIT_FAILURE);
  }

  return listen_sock;
}

void listen(int socket) {
  sockaddr_in server_addr = {
      .sin_family = AF_INET,
      .sin_port = htons(9000)
  };

  inet_pton(AF_INET, "0.0.0.0", &server_addr.sin_addr);

  if (bind(socket, (sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
    perror("[ERROR:2] Socket cannot be created!");
    exit(EXIT_FAILURE);
  }

  if (listen(socket, SOMAXCONN) < 0) {
    perror("[ERROR:3] Socket cannot be switched to listen mode!");
    exit(EXIT_FAILURE);
  }
}

int main() {
  auto eventloop = std::make_shared<oxm::EventLoop>();

  int listen_sock = createSocket();
  listen(listen_sock);


  auto event = oxm::Event();
  event.TriggerOn(oxm::Event::Read);
  event.fd = listen_sock;

  auto listenEventId = eventloop->RegisterEvent(event);

  auto task1 = eventloop->CreateTask([eventloop, listen_sock, listenEventId](oxm::Event::Mask mask) {
    sockaddr_in client_addr;
    socklen_t client_addr_size = sizeof(client_addr);
    int conn_sock = accept(listen_sock, (sockaddr *)&client_addr, &client_addr_size);
    if (conn_sock == -1) {
      perror("[ERROR:7] accept");
      exit(EXIT_FAILURE);
    }

    if (fcntl(conn_sock, F_SETFL, O_NONBLOCK) == -1) {
      perror("[ERROR:8] fcntl");
      exit(EXIT_FAILURE);
    }

    auto event = oxm::Event();
    event.TriggerOn(oxm::Event::Read);
    event.fd = conn_sock;

    auto eventId = eventloop->RegisterEvent(event);

    auto task2 = eventloop->CreateTask([eventloop, conn_sock, eventId](oxm::Event::Mask mask) {
      if (oxm::HasError(mask)) {
        perror("[ERROR:9] Error");
        eventloop->Unshedule(eventId, true);
        return;
      }

      if (oxm::CanRead(mask)) {
        char buffer[4096];
        int len = read(conn_sock, &buffer, 4096);
        if (len > 0) {
          write(1, buffer, len);
        }
      }
    });

    eventloop->Bind(eventId, task2);
    eventloop->Schedule(eventId);
  });

  eventloop->Bind(listenEventId, task1);
  eventloop->Schedule(listenEventId);

  while (true) {
    eventloop->Poll();
  }
}