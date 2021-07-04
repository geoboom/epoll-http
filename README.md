# Epoll HTTP Server

A simple single threaded, epoll-based HTTP server written in C++.

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
mkdir build && cd build
cmake .. && make
```

This builds the binary `server` which you can run with `./server` in the `build/` folder. It also builds the static library `server_lib` which is linked to the test binary `server_test`. You may run the unit tests (using GoogleTest) with the `ctest` command in the `build/` folder.

The server may be visited in your browser through `http://localhost:8080/`. Several endpoints are available like `/`,  `/dtl`, `/home`, `/echo`.

In particular, `/echo` echoes back to the client the URL request parameters, along with the json data in the body if the request is a POST request of content type `application/json`. To see this in action, you may either visit `http://localhost:8080/echo?param1=xyz&param2=def` or run the following command:

```sh
curl \
--header 'Content-Type: application/json' \
--request POST \
--data '{ "username": "geoboom", "password": "dytechlab" }' \
'localhost:8080/echo?param1=xyz&param2=def'
```

You should see the same data echoed back to you.

**Note:** you may change the port on which the server is run (`8080` by default) in `src/http_server.h`, under the `DEFAULT_PORT` definition.

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

As for features, HTTP GET and POST requests with url query parameters and JSON POST body are supported. There is also minimal exception handling due to time constraints, though I would like to add a general `HttpException` class that can be thrown and caught by the handler which will generate an appropriate HTTP response object if I had more time. 

To begin, you could take a look at `main.cpp` to see how the HTTP resource endpoints along with their route handlers are created.
