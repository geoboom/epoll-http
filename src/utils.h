#pragma once
#include "http_server.h"
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
#include <vector>

namespace geo {
  const std::string WHITESPACE = " \n\r\t\f\v";

  inline std::string ltrim(const std::string &s)
  {
    size_t start = s.find_first_not_of(WHITESPACE);
    return (start == std::string::npos) ? "" : s.substr(start);
  }

  inline std::string rtrim(const std::string &s)
  {
    size_t end = s.find_last_not_of(WHITESPACE);
    return (end == std::string::npos) ? "" : s.substr(0, end + 1);
  }

  inline std::string trim(const std::string &s) {
    return rtrim(ltrim(s));
  }

  using KVMap = std::unordered_map<std::string, std::string>;

}

const std::string LINE_SEP = "\r\n";

