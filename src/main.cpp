#include "http_server.h"
#include "iostream"
#include "fstream"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

void get_post_echo(geo::Request &request, geo::Response &response) {
  geo::KVMap data = request.get_data();
  geo::KVMap url_params = request.get_url_params();

  json v;
  for (auto &it : data) {
    v["data_received"][it.first] = it.second;
  }
  for (auto &it : url_params) {
    v["url_params_received"][it.first] = it.second;
  }

  response.set_data(v.dump());
}

void get_home(geo::Request &request, geo::Response &response) {
  response.set_data("Hello there!\n");
}

void get_index(geo::Request &request, geo::Response &response) {
  std::ifstream ifs("../public/index.html");
  std::string content((std::istreambuf_iterator<char>(ifs)),
                      (std::istreambuf_iterator<char>()));

  response.set_data(content);
  response.set_header("Content-Type", "text/html");
}

void get_dtl(geo::Request &request, geo::Response &response) {
  std::ifstream ifs("../public/dtl.html");
  std::string content((std::istreambuf_iterator<char>(ifs)),
                      (std::istreambuf_iterator<char>()));

  response.set_data(content);
  response.set_header("Content-Type", "text/html");
}

int main() {
  geo::HttpServer server;

  // ===============================
  // resources setup BEGIN
  // ===============================
  geo::HttpResource echo_resource;
  echo_resource.add_handler(geo::HttpMethod::POST, get_post_echo);
  echo_resource.add_handler(geo::HttpMethod::GET, get_post_echo);
  server.add_resource("/echo", echo_resource);

  geo::HttpResource home_resource;
  home_resource.add_handler(geo::HttpMethod::GET, get_home);
  server.add_resource("/home", home_resource);

  geo::HttpResource index_resource;
  index_resource.add_handler(geo::HttpMethod::GET, get_index);
  server.add_resource("/", index_resource);

  geo::HttpResource dtl_resource;
  dtl_resource.add_handler(geo::HttpMethod::GET, get_dtl);
  server.add_resource("/dtl", dtl_resource);
  // ===============================
  // resources setup END
  // ===============================

  server.start();
  return 0;
}
