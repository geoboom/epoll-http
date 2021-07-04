#include <nlohmann/json.hpp>
#include "utils.h"
#include "connection_context.h"
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


void geo::Request::parse_request(const std::string &raw) {
  size_t pos_request_line_end = raw.find(LINE_SEP);
  if (pos_request_line_end == std::string::npos) {
    // throw error
    return;
  }
  size_t pos_headers_end = raw.find(LINE_SEP + LINE_SEP, pos_request_line_end);
  size_t pos_headers_start = pos_request_line_end + LINE_SEP.size();
  std::string request_line = raw.substr(0, pos_request_line_end),
              headers = raw.substr(pos_headers_start, pos_headers_end - pos_headers_start);

  parse_request_line(request_line);
  parse_headers(headers);
  if (_method == geo::HttpMethod::POST) {
    size_t pos_body_start = pos_headers_end + (LINE_SEP + LINE_SEP).size();
    size_t body_length = raw.size() - pos_body_start + 1;
    if (_content_length < body_length) {
      // throw error
    }
    parse_body(raw.substr(pos_body_start, _content_length));
  }
}

const void geo::Request::parse_headers(const std::string& headers) {
  size_t prev = 0, cur;
  while (cur != std::string::npos) {
    cur = headers.find(LINE_SEP, prev);
    size_t pos_sep = headers.find(":", prev);
    std::string key = geo::trim(headers.substr(prev, pos_sep - prev)),
                value = geo::trim(headers.substr(pos_sep + 1, cur - pos_sep - 1));
    _headers.emplace(std::move(key), std::move(value));
    prev = cur + 1;
  }
  set_special_headers();
}

const void geo::Request::set_special_headers() {
  _keep_alive = _headers.count("Connection") && _headers["Connection"] == "Keep-Alive";
  _content_type = _headers["Content-Type"];
  _content_length = _headers.count("Content-Length") ? std::stoi(_headers["Content-Length"]) : 0;
}

const void
geo::Request::parse_request_line(const std::string &request_line) {
  // extract method, path, protocol
  size_t pos_method_end = request_line.find(" ");
  size_t pos_path_end = request_line.find(" ", pos_method_end + 1);
  size_t pos_protocol_end = request_line.find(" ", pos_path_end + 1);
  _method_string = request_line.substr(0, pos_method_end);
  if (!geo::method_to_enum.count(_method_string)) {
    // throw error
  }
  _method = geo::method_to_enum[_method_string];
  _url = request_line.substr(pos_method_end + 1,
                              pos_path_end - pos_method_end - 1);
  std::string protocol = request_line.substr(pos_path_end + 1,
                                  request_line.size() - pos_path_end - 1);
  if (protocol != "HTTP/1.1") {
    // throw error
  }
  size_t url_params_start = _url.find("?");
  _path = _url.substr(0, url_params_start);
  if (url_params_start == std::string::npos) { // no params
    return;
  }

  size_t params_length = _url.size() - url_params_start;
  std::istringstream params(_url.substr(url_params_start + 1, params_length));
  std::string kvpair;
  while(std::getline(params, kvpair, '&')) {
    size_t pos_sep = kvpair.find("=");
    std::string key = kvpair.substr(0, pos_sep), value = kvpair.substr(pos_sep + 1);
    _url_params[key] = value;
  }
}

const void geo::Request::parse_body(const std::string& body) {
  using json = nlohmann::json;
  try {
    json json_data = json::parse(body);
    for(auto it = json_data.begin(); it != json_data.end(); ++it) {
      _data[it.key()] = it.value();
    }
  } catch (json::parse_error& e) {
    std::cout << "[ERROR] Error parsing JSON body of POST request." << '\n';
    return;
  }
}

std::ostream &geo::operator<<(std::ostream &os, const geo::Request &request) {
  os << "METHOD: " << request._method << '\n';
  os << "PATH: " << request._url << '\n';
  os << "BODY: " << request._body << '\n';
  os << "CONTENT LENGTH: " << request._content_length << '\n';
  os << "CONTENT TYPE: " << request._content_type;
  return os;
}

const geo::KVMap &geo::Request::get_url_params() { return _url_params; }

const geo::KVMap &geo::Request::get_data() { return _data; }

const int geo::Request::get_content_length() const { return _content_length; }

const std::string &geo::Request::get_content_type() { return _content_type; }

const std::string &geo::Request::get_method_string() { return _method_string; }

const geo::HttpMethod geo::Request::get_method() const { return _method; }

const std::string &geo::Request::get_raw_body() { return _body; }

const std::string &geo::Request::get_url() { return _url; }

const std::string &geo::Request::get_path() { return _path; }

const bool geo::Request::is_valid() const { return _valid; }

const bool geo::Request::is_keep_alive() const { return _keep_alive; };

geo::ClientContext::ClientContext(int fd, const std::string& ip_addr)
  : fd(fd), ip_addr(ip_addr) {
  }

geo::Request& geo::ClientContext::get_request() {
  return _request;
}

geo::Response& geo::ClientContext::get_response() {
  return _response;
}

void geo::Response::set_header(const std::string &key, const std::string &value) {
  _headers[key] = value;
}

void geo::Response::set_status_code(int status_code) {
  _status_code = status_code;
}

void geo::Response::set_status_message(const std::string& status_message) {
  _status_message = status_message;
}

void geo::Response::set_data(const std::string& data) {
  _data = data;
}

void geo::Response::prepare(bool keep_alive) {
  std::ostringstream oss;
  oss << "HTTP/1.1" << " " << _status_code << " " << _status_message << LINE_SEP;
  if (!_headers.count("Content-Type")) {
    oss << "Content-Type: application/json" << LINE_SEP;
  }
  if (!_headers.count("Content-Length")) {
    oss << "Content-Length: " << _data.size() << LINE_SEP;
  }
  oss << "Connection: " << (keep_alive ? "Keep-Alive" : "close") << LINE_SEP;
  for(auto &it : _headers) {
    oss << it.first << ": " << it.second << LINE_SEP;
  }
  oss << LINE_SEP;
  oss << _data;
  _response_string = oss.str();
}

std::string& geo::Response::get_response_string(bool keep_alive) {
  if (!_prepared) {
    prepare(keep_alive);
    _prepared = true;
  }
  /* std::cout << _response_string << '\n'; */
  return _response_string;
}
