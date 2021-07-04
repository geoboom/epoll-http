#pragma once
#include "connection_context.h"
#include "handler.h"
#include <arpa/inet.h>
#include <cerrno>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <memory>
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

#define MAX_EVENTS 10500 // to handle c10k

namespace geo {
class EpollLoop {
public:
  static EpollLoop &Instance() {
    static EpollLoop instance;
    return instance;
  }
  void set_tcp_fd(const int fd);
  void setnonblocking(const int fd);
  void start();
  Handler* handler;

protected:
  EpollLoop();
  ~EpollLoop();
  EpollLoop(const EpollLoop &) = delete;
  EpollLoop &operator=(const EpollLoop &) = delete;

private:
  int _epfd, _tcp_fd;
};
} // namespace geo
