#pragma once
#include <arpa/inet.h>
#include <cerrno>
#include <climits>
#include <cstdio>
#include <cstdlib>
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
#include <vector>

#include <unordered_map>

namespace geo {
class Handler {
public:
  virtual void handle_read(epoll_event &event) = 0;
  virtual void handle_write(epoll_event &event) = 0;
  virtual void handle_close(const int fd) = 0;
  virtual std::pair<int, std::string> handle_accept(epoll_event &event) = 0;
};
} // namespace geo
