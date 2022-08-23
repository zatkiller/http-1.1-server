# HTTP 1.1 server

## Contents

* [HTTP 1.1 server](#http-11-server)
  * [Task](#task)
    * [Requirements](#task-requirements)
    * [Supported Features](#supported-features)
  * [Repository Structure](#repository-structure)
  * [Design](#design)
  * [Setup](#setup)
      * [Prerequisites](#prerequisites)
      * [Build and run server](#build-and-run-server)
  * [Testing](#testing)
      * [Curl command](#curl-commands)
      * [Benmarking performance](#benchmarking)

## Task
Implement a basic HTTP server that supports HTTP/1.1 in C++.

### Task Requirements
- Run on Linux
- Do not use any third party networking library, i.e. use only Linux system API.
- Handle at least 10,000 concurrent connections.
- Serve at least 100,000 requests per second on a modern personal computer.
- Serve a simple one or two pages website for demonstration.
- You may want to skip some trivial features like Multipart data if time is not enough, but you need to state clearly what features are supported.

### Supported Features
- HTTP/1.1 GET requests to the following endpoints
  - `/` - returns a dummy web page
  - `/hello` - returns hello world as response body
  - `/echo`- returns response containing the request query parameters data
  - `/fibonacci` - returns the response of the fibonacci seqeuence using the number in the request query parameter
- HTTP/1.1 POST requests to the following endpoints
  - `/echo`- returns response containing the request body data
  - `/fibonacci` - returns the response of the fibonacci seqeuence using the number in the request body

## Repository Structure
- `event_data`
  - contains the `EventData` class which is contains the `Request` and `Response` associated with the event.
- `event_loop`
  - contains the `EventLoop` which is responsible for accepting new client connections and dispatching the request handlers on worker threads
- `http_message`
  - contains the `HttpRequest`, `HttpResponse` classes and other helper enums that is used to represent the HTTP requests and responses
- `http_resource`
  - contains the `HttpResource` class which stores the route handlers
- `http_server`
  - contains the `HttpServer` class which inherits from the EventHandler parent class to handle events
- `main`
  - Main program where the `HttpServer` and endpoints are initialized
- `utils`
  - Helper functions for string parsing and logging

## Design
- Proactor design pattern
- 1 Listener thread with busy poll accept to accept incoming client connections
- 5 Worker threads with epoll loops to process requests and responses

The initial design of the HTTP server used the Reactor Pattern. The reactor design pattern is an event handling pattern for handling service requests delivered concurrently to a service handler by one or more inputs. The service handler then demultiplexes the incoming requests and dispatches them synchronously to the associated request handlers. 

During the initial benchmarking with the Reactor Pattern, the HTTP server could support the 10k concurrent client connections but could not support the 100k requests/s throughput requirement, only achieving 40-50k requets/s. This is likely due to the server running the request handlers synchronously and as a result, computational heavy requests slow down the total throughput of the server.

To increase the throughput of the server, I decided to switch to a Proactor pattern that is essentially an asynchronous variant of the Reactor pattern. It is also adopted by Boost Asio. It dispatches request handlers asynchronously on a worker thread without blocking the event loop, increasing the throughput of the HTTP server. With the Proactor Pattern, the HTTP server was able to support > 100k requests per second.


### References:
- Reactor Pattern: https://en.wikipedia.org/wiki/Reactor_pattern
- Proactor Pattern: https://en.wikipedia.org/wiki/Proactor_pattern


## Setup

### Prerequisites
To set up this repository, you will need:
- `cmake` with version `3.23` or higher
- Compiler that supports `C++17
- `nlohmann/json` which is used to for JSON parsing. It is already inlcuded under the `include` directory, but can be reinstalled using `wget https://github.com/nlohmann/json/releases/download/v3.9.1/json.hpp -P include/nlohmann/`

### Build and run server
1. `cd` into project root folder
2. Run `mkdir build` to create build directory 
3. Run `cd build && cmake .. && make -j4` to build executable using cmake under `build` dir
4. Run the executable using the command `./http_server`

## Testing

### CURL commands

The following CURL commands can be run to test the HTTP Endpoints

#### GET
```
curl "localhost:8080"
curl "localhost:8080/hello"
```

#### POST

```
curl --header 'Content-Type: application/json' --request POST --data '{ "username": "zatkiller", "password": "123456" }' 'localhost:8080/echo?param1=xyz&param2=def'
curl --header 'Content-Type: application/json' --request POST --data '{ "number": "123" }' 'localhost:8080/fibonacci'
```

### Benchmarking

Benchmarked using wrk tool on linux (WSL) environment

#### Results with single-threaded

```
wrk -t5 -c10000 -d5s --latency http://localhost:8080/hello

Running 5s test @ http://localhost:8080/hello
  5 threads and 10000 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency   203.31ms   25.77ms   1.96s    91.07%
    Req/Sec     9.67k     3.72k   19.26k    68.62%
  Latency Distribution
     50%  200.33ms
     75%  214.01ms
     90%  224.65ms
     99%  230.50ms
  230543 requests in 5.07s, 11.21MB read
  Socket errors: connect 0, read 0, write 0, timeout 27
Requests/sec:  45446.37
Transfer/sec:      2.21MB
```

#### Results with thread pool (5 workers + main thread busy-polling `accept(...)`)

```
wrk -t5 -c10000 -d5s --latency http://localhost:8080/hello
Running 5s test @ http://localhost:8080/hello
  5 threads and 10000 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency    61.02ms    7.63ms 102.32ms   74.20%
    Req/Sec    32.53k     2.20k   41.20k    83.33%
  Latency Distribution
     50%   62.72ms
     75%   66.59ms
     90%   68.79ms
     99%   70.91ms
  776654 requests in 5.09s, 37.77MB read
Requests/sec: 152728.13
Transfer/sec:      7.43M
```
