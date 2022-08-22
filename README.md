# HTTP 1.1 server

## Features
- HTTP 1.1 server
- Able to handle up to 10k concurrent connections
- Able to support > 100k requests/s

## Design
- Proactor design pattern
- 1 Listener thread with busy poll accept to accept incoming client connections
- 5 Worker threads with epoll loops to process requests and responses

## Build and run server

Create build directory

```
mkdir build
```

Build with cmake

```
cd build && cmake .. && make -j4
```

Running server

```
./http_server
```

## CURL commands

GET

```
curl "localhost:8080"
curl "localhost:8080/hello"
```

POST

```
curl --header 'Content-Type: application/json' --request POST --data '{ "username": "zatkiller", "password": "123456" }' 'localhost:8080/echo?param1=xyz&param2=def'
curl --header 'Content-Type: application/json' --request POST --data '{ "number": "123" }' 'localhost:8080/fibonacci'
```
## Benchmarking

Benchmarked using wrk tool

### Results with single-threaded

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

### Results with thread pool (5 workers + main thread busy-polling `accept(...)`)

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
