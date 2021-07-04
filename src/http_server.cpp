#include "http_server.h"
#include "utils.h"
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

sockaddr_in geo::HttpServer::create_listening_addr() {
  sockaddr_in ret;
  memset(&ret, 0, sizeof(ret));
  ret.sin_family = AF_INET;
  ret.sin_port = htons(_port);
  ret.sin_addr.s_addr = htonl(INADDR_ANY);
  return ret;
}

geo::HttpServer::HttpServer(int port)
    : _port(port), _epoll_loop(geo::EpollLoop::Instance()) {
  _tcp_fd = socket(AF_INET, SOCK_STREAM, 0);
  int opt = 1;
  setsockopt(_tcp_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
  sockaddr_in addr{create_listening_addr()};
  bind(_tcp_fd, (sockaddr *)&addr, sizeof(addr));
  listen(_tcp_fd, LISTEN_BACKLOG);
  std::cout << "[INFO] Server running at: " << inet_ntoa(addr.sin_addr) << ":"
            << ntohs(addr.sin_port) << '\n';
  _epoll_loop.set_tcp_fd(_tcp_fd);
  _epoll_loop.handler = dynamic_cast<Handler*>(this);
}

void geo::HttpServer::start() { _epoll_loop.start(); }

std::pair<int, std::string> geo::HttpServer::handle_accept(epoll_event &event) {
  int fd = event.data.fd;
  sockaddr_in addr;
  socklen_t addr_len = sizeof(addr);
  int client_fd = accept(fd, (sockaddr *)&addr, &addr_len);
  /* std::cout << "[INFO] handle_accept: client_fd=" << client_fd */
  /*           << " client_ip=" << inet_ntoa(addr.sin_addr) << '\n'; */
  return std::make_pair(client_fd, inet_ntoa(addr.sin_addr));
}

void geo::HttpServer::handle_read(epoll_event &event) {
  ClientContext *client_context = (ClientContext *)event.data.ptr;
  char buffer[BUFFER_SIZE];
  memset(buffer, 0, BUFFER_SIZE);
  int fd = client_context->fd;
  read(fd, buffer, BUFFER_SIZE);
  client_context->get_request().parse_request(buffer);
  route_request(client_context->get_request(), client_context->get_response());
}

void geo::HttpServer::route_request(Request& request, Response& response) {
  if (!_resource_map.count(request.get_path())) {
    response.set_status_code(404);
    response.set_status_message("Not Found");
    response.set_data("Path='" + request.get_path() + "' not found.");
    return;
  }

  HttpResource resource = _resource_map[request.get_path()];
  if (!resource.method_allowed(request.get_method())) {
    response.set_status_code(405);
    response.set_status_message("Not Allowed");
    response.set_data("Path='" + request.get_path() +
                      "' does not allow HttpMethod='" +
                      request.get_method_string() + "'.");
    return;
  }

  // simple error handling 
  try {
    resource.handle(request, response);
    response.set_status_code(200);
    response.set_status_message("OK");
  } catch (...) {
    response.set_status_code(500);
    response.set_status_message("Internal Server Error");
  }
}

void geo::HttpServer::handle_write(epoll_event &event) {
  ClientContext *client_context = (ClientContext *)event.data.ptr;
  std::string response_string =
      client_context->get_response().get_response_string(
          client_context->get_request().is_keep_alive());
  write(client_context->fd, response_string.data(), response_string.size());
}

void geo::HttpServer::handle_close(const int fd) {
  close(fd);
}

bool geo::HttpResource::method_allowed(geo::HttpMethod method) {
  return 0 != _method_to_handler.count(method);
}

void geo::HttpResource::handle(Request& request, Response& response) {
  _method_to_handler[request.get_method()](request, response);
}

void geo::HttpResource::add_handler(geo::HttpMethod method, const geo::RouteHandler& route_handler) {
  _method_to_handler[method] = route_handler;
}

void geo::HttpServer::add_resource(const std::string& path, const HttpResource& resource) {
  _resource_map[path] = resource;
}

