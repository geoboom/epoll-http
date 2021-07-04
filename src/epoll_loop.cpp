#include "epoll_loop.h"
#include <arpa/inet.h>
#include <cerrno>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <map>
#include <netinet/in.h>
#include <set>
#include <signal.h>
#include <sstream>
#include <string.h>
#include <sys/epoll.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <sys/sysinfo.h>
#include <sys/time.h>
#include <unistd.h>
#include <unordered_map>
#include <vector>

geo::EpollLoop::EpollLoop() { _epfd = epoll_create1(0); }

void geo::EpollLoop::start() {
  epoll_event events[MAX_EVENTS];
  std::cout << "[INFO] epoll loop start" << '\n';
  for (;;) {
    // should probably use a logger class for this
    /* std::cout << "[INFO] epoll loop waiting" << '\n'; */
    /* int nfds = epoll_wait(_epfd, events, MAX_EVENTS, -1); */
    int nfds = epoll_wait(_epfd, events, MAX_EVENTS, -1);
    /* std::cout << "[INFO] epoll loop got events: nfds=" << nfds << '\n'; */
    for (int i = 0; i < nfds; ++i) {
      epoll_event &event = events[i];
      /* std::cout << "event.data.fd = " << event.data.fd << " -> "; */
      if (event.data.fd == _tcp_fd) {
        /* std::cout << "(1) _tcp_fd" << '\n'; */
        auto [client_fd, client_ip] = handler->handle_accept(event);
        setnonblocking(client_fd);
        epoll_event nevent;
        nevent.events = EPOLLIN | EPOLLET;
        nevent.data.ptr = new ClientContext(client_fd, client_ip);
        epoll_ctl(_epfd, EPOLL_CTL_ADD, client_fd, &nevent);
      } else if (event.events & EPOLLIN) {
        /* std::cout << "(2) EPOLLIN" << '\n'; */

        // TODO: delegate computation to thread pool to increase throughput (QPS)
        handler->handle_read(event);

        // assume read is completely over, though may not be the case in ET
        epoll_event nevent;
        nevent.events = EPOLLOUT | EPOLLET;
        epoll_ctl(_epfd, EPOLL_CTL_MOD, ((ClientContext *)event.data.ptr)->fd,
                  &nevent);
      } else if (event.events & EPOLLOUT) {
        /* std::cout << "(3) EPOLLOUT" << '\n'; */

        // TODO: delegate computation to thread pool to increase throughput (QPS)
        handler->handle_write(event);
        
        // assume write is completely over, though may not be the case in ET
        int fd = ((ClientContext *)event.data.ptr)->fd;
        epoll_ctl(_epfd, EPOLL_CTL_DEL, fd, NULL);
        close(fd);
      } else {
        std::cout << "(4) ???" << '\n';
      }
    }
  }
}

void geo::EpollLoop::setnonblocking(const int fd) {
  int old_flags = fcntl(fd, F_GETFL, 0);
  fcntl(fd, F_SETFL, old_flags | O_NONBLOCK);
}

void geo::EpollLoop::set_tcp_fd(const int fd) {
  setnonblocking(fd);

  epoll_event event;
  event.events = EPOLLIN | EPOLLET;
  event.data.fd = fd;
  epoll_ctl(_epfd, EPOLL_CTL_ADD, fd, &event);

  _tcp_fd = fd;
}

geo::EpollLoop::~EpollLoop() { free(handler); }
