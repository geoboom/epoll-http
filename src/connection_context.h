#pragma once
#include "http_methods.h"
#include "utils.h"
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
#include <vector>

namespace geo {

using KVMap = std::unordered_map<std::string, std::string>;

class Response {
  friend std::ostream &operator<<(std::ostream &os, const Response &response);
  friend class ClientContext;

private:
  KVMap _headers;
  std::string _data, _status_message, _response_string;
  int _status_code;
  bool _prepared;

protected:
  Response() = default;

public:
  void set_header(const std::string &key, const std::string &value);
  void set_status_code(int status_code);
  void set_status_message(const std::string& status_message);
  void set_data(const std::string& data);
  void prepare(bool keep_alive);
  std::string &get_response_string(bool keep_alive);
};

class Request {
  friend std::ostream &operator<<(std::ostream &os, const Request &request);
  friend class ClientContext;

private:
  HttpMethod _method;
  std::string _url;
  std::string _path;
  std::string _body;
  std::string _content_type;
  std::string _method_string;
  KVMap _headers, _data, _url_params;
  int _content_length;
  int _fd;
  bool _valid;
  bool _done;
  bool _keep_alive;

  const void set_special_headers();
  const void parse_request_line(const std::string &request_line);
  const void parse_headers(const std::string &headers);
  const void parse_body(const std::string &body);

protected:
  Request() = default;

public:
  void parse_request(const std::string &raw);

  const std::string& get_method_string();
  const HttpMethod get_method() const;

  const std::string &get_url();
  const std::string &get_path();
  const std::string &get_raw_body();
  const std::string &get_content_type();
  const KVMap &get_url_params();
  const KVMap &get_data();

  const int get_content_length() const;
  const bool is_valid() const;
  const bool is_keep_alive() const;
};

std::ostream &operator<<(std::ostream &os, const Request &request);

class ClientContext {
public:
  ClientContext(int fd, const std::string& ip_addr);
  void *ptr;
  int fd;
  std::string ip_addr;
  Response &get_response();
  Request &get_request();

private:
  Response _response;
  Request _request;
};

} // namespace geo
