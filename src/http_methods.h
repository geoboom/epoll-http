#pragma once
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
#include <unordered_map>

namespace geo {

enum HttpMethod { OPTIONS = 0, GET, POST, SIZE };

static std::unordered_map<std::string, HttpMethod> method_to_enum{
    {"OPTIONS", OPTIONS},
    {"GET", GET},
    {"POST", POST},
};

} // namespace geo
