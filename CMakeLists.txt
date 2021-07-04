cmake_minimum_required(VERSION 3.7)
project(epoll-http-server)

set(CMAKE_CXX_STANDARD 17)	
set(BINARY server)

enable_testing()

include_directories(
    src
    include
    )

file(GLOB sources src/*.cpp src/*.h)
add_executable(${BINARY} ${sources})	
add_library(${BINARY}_lib STATIC ${sources})

add_subdirectory(test) # not quite updated

