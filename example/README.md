# SimpleSACN Usage Example

## Purpose

Demonstrates usage of SimpleSACN classes

## Requirements

Host machine in the same network as an active sACN source broadcasting onto universe 1

## Building

```pwsh
cd SimpleSACN/example
cmake -B build
cmake --build build
./build/Debug/SimpleSACN_Example.exe
```

## Dependencies

chriskohlhoff/asio#asio-1-39-0 - Networking and timing
