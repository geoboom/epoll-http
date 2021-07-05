# Epoll HTTP Server

A simple single threaded, epoll-based HTTP server written in C++.

## Contents

* [Epoll HTTP Server](#epoll-http-server)
  * [Setup guide](#setup-guide)
    * [Prerequisites](#prerequisites)
    * [Building and running](#building-and-running)
  * [Example usage](#example-usage)
  * [Design notes](#design-notes)
  * [Performance benchmarks](#performance-benchmarks)
  * [TODO](#todo)

## Setup guide

### Prerequisites

You will need `cmake` with version at least `3.7`, and a compiler with `C++17` support.

You will also need `nlohmann/json` which can be installed with the following command, ran in the project root:

```sh
wget https://github.com/nlohmann/json/releases/download/v3.9.1/json.hpp -P include/nlohmann/
```

### Building and running

In the project root, run:

```sh
mkdir build && cd build && cmake .. && make -j4
```

This builds the binary `server` which you can run with `./server` in the `build/` folder. It also builds the static library `server_lib` which is linked to the test binary `server_test`. You may run the unit tests (using GoogleTest) with the `ctest` command in the `build/` folder.

The server may be reached in your browser through `http://localhost:8080/`. Several endpoints are available like `/`,  `/dtl`, `/home`, `/echo`.

In particular, `/echo` echoes back to the client the URL request parameters, along with the json data in the body if the request is a POST request of content type `application/json`. To see this in action, you may either visit `http://localhost:8080/echo?param1=xyz&param2=def` or spawn another terminal instance and run the following command:

```sh
curl \
--header 'Content-Type: application/json' \
--request POST \
--data '{ "username": "geoboom", "password": "dytechlab" }' \
'localhost:8080/echo?param1=xyz&param2=def'
```

You should see the same data echoed back to you.

**Note:** you may change the port on which the server is run (`8080` by default) in `src/http_server.h`, under the `DEFAULT_PORT` definition.

## Example usage

You may find this in `src/main.cpp`.

```cpp
#include "http_server.h"
#include "iostream"
#include "fstream"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

void get_post_echo(geo::Request &request, geo::Response &response) {
  geo::KVMap data = request.get_data();
  geo::KVMap url_params = request.get_url_params();

  json v;
  for (auto &it : data)
    v["data_received"][it.first] = it.second;
  for (auto &it : url_params)
    v["url_params_received"][it.first] = it.second;

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
```


## Design notes

Following is the directory tree as seen from the root.

```sh
.
├── build
│   ├── ...
│   └── server
├── CMakeLists.txt
├── include
│   └── nlohmann
│       └── json.hpp
├── public
│   ├── dtl.html
│   └── index.html
├── README.md
├── src
│   ├── connection_context.cpp
│   ├── connection_context.h
│   ├── epoll_loop.cpp
│   ├── epoll_loop.h
│   ├── handler.h
│   ├── http_methods.h
│   ├── http_server.cpp
│   ├── http_server.h
│   ├── main.cpp
│   └── utils.h
└── test
    ├── CMakeLists.txt
    └── tests.cpp
```

Overview:

- `src/` contains all the project source files and `test/` contains the unit test files.
- `connection_context` contains the classes `Request`, `Response`, and `ClientContext` which manage the state of a client's connection. Also contains functions that parse HTTP request data and generate HTTP responses.
- `epoll_loop` contains the `EpollLoop` class which is a singleton that manages the IO loop of the server in an asynchronous, nonblocking fashion using the `epoll` syscall.
- `handler` contains the `Handler` interface which defines a few interface methods to handle `epoll_event`s.
- `http_server` contains the `HttpServer` class which implements the `Handler` interface to handle accept, read, write events. It also contains `HttpResource` class which encapsulates a HTTP resource.
- `http_methods`, `utils` contain some helper `enum`s and functions.
- `main.cpp` is where the `HttpServer` and endpoints are set up.

The design of my server follows the reactor design pattern, which is an event handling pattern for handling service requests delivered concurrently to a service handler by one or more inputs (clients on a socket). The service handler then demultiplexes the incoming requests and dispatches them synchronously to the associated request handlers. This is similar to how NodeJS handles events on a single thread and dispatches CPU intensive tasks to a worker thread pool, except that for my server, there is only one thread; I did not have time to explore this worker thread pool enhancement.

As for features, HTTP GET and POST requests with url query parameters and JSON POST body are supported; HTTP/1.1 "Connection: Keep-Alive" header for persistent connection is also supported. There is minimal exception handling due to time constraints, though I would like to add a general `HttpException` class that can be thrown and caught by the handler which will generate an appropriate HTTP response object if I had more time. Also, requests / responses will truncate to `BUFFER_SIZE` when reading from / writing to client socket as I did not find supporting and testing larger request sizes to be a priority under the time constraints.

To begin, you could take a look at `main.cpp` to see how the HTTP resource endpoints along with their route handlers are created.

## Performance benchmarks

Using wrk, a HTTP benchmarking tool (https://github.com/wg/wrk), I managed to get 8k - 10k queries per second (QPS) with 10k concurrent connections on my main computer which runs an old Intel Core i7-4930K (6C12T@3.90GHz).

Unfortunately, I wasn't able to hit 100k QPS throughput with a single thread like I had anticipated at the start of this project. I realized this too late and figured that I would need a worker thread pool to achieve 100k QPS, but I did not have enough time to implement it. Nonetheless, it was a very enjoyable experience researching and working on this project and I am happy with the results (vs 12k QPS on barebones NodeJS server). Of course, many aspects of the server could be optimized with more time.

The command for the benchmark is:
```sh
wrk -c10k -d10s http://localhost:8080/home
```

You can install wrk via:
```sh
sudo apt update && sudo apt install wrk
```

## TODO

Improve throughput (QPS) by delegating requests to a pre-initialized worker thread pool.

