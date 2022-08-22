cmake_minimum_required(VERSION 3.23)
project(http-server)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED on)
set(BINARY http_server)
SET(GCC_P_THREAD_COMPILE_FLAG "-pthread")
SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_P_THREAD_COMPILE_FLAG}")

include_directories(src include)

file(GLOB_RECURSE srcs src/*.cpp src/*.h)
add_executable(${BINARY} ${srcs})