#pragma once
#include "handler.h"
#include "epoll_loop.h"
#include "http_methods.h"
#include <arpa/inet.h>
#include <cerrno>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <functional>
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
#include <unordered_set>
#include <vector>

#define LISTEN_BACKLOG 1024
#define DEFAULT_PORT 8080
#define BUFFER_SIZE 1024

namespace geo {

class Request;
class Response;
class EpollLoop;

using RouteHandler = std::function<void(Request&, Response&)>;

class HttpResource {
  friend class HttpServer;

private:
  std::unordered_map<HttpMethod, RouteHandler> _method_to_handler;

public:
  bool method_allowed(HttpMethod method);
  void handle(Request& request, Response& response);
  void add_handler(HttpMethod method, const RouteHandler& route_handler);
};

class HttpServer : public Handler {
public:
  HttpServer(int port = DEFAULT_PORT);
  void start();

  void handle_read(epoll_event &event) override;
  void handle_write(epoll_event &event) override;
  void handle_close(const int fd) override;
  std::pair<int, std::string> handle_accept(epoll_event &event) override;

  void route_request(Request& request, Response& response);
  void add_resource(const std::string& path, const HttpResource& resource);

private:
  sockaddr_in create_listening_addr();
  int _tcp_fd, _port;
  std::unordered_map<std::string, HttpResource> _resource_map; // path -> resource
  EpollLoop &_epoll_loop;
};

} // namespace geo
