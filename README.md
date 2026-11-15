
# SimpleSACN

*A header-only library implementing the ANSI E1.31 sACN standard, allowing for the conversion of sACN network packets into DMX frames following the sACN standard. Bytes in -> DMX out. Users are free to focus on application logic.*

## Platforms

This library is tested with the C++17 standard. It should be noted that the SafeNode header file providing a simple sACN->DMX node depends on the target platform providing an implementation of std::mutex. The SafeNode class is essentially a thread-safe wrapper of SacnReceiver, intended to provide a simple worry free solution to handle network input and hardware output. SacnReceiver can be used directly if the SafeNode class is not usable on your target platform.

## Getting Started

Clone this repo and then either

Manual: Copy src/ into your >= C++17 project or add it to your include path; include "SimpleSACN.hpp" for everything, or only include the specific headers you want.

CMake: The library is the target `SimpleSACN::SimpleSACN`

```cmake
add_subdirectory(relative/path/to/SimpleSACN)

target_link_libraries(your_app PRIVATE SimpleSACN::SimpleSACN)
```

```cpp
#include <SimpleSACN.hpp>
```

PlatformIO: add the repo to `platformio.ini`

```ini
lib_deps = https://github.com/jking100/SimpleSACN.git
```

## Example: ESP32 Wireless Node

[SimpleSACN-ESP32-Node](https://github.com/jking100/SimpleSACN-ESP32-Node): WiFi sACN to DMX node firmware for a Feather ESP32-S3 + M5 DMX unit, built on SafeNode.

## Easy Usage

`Node::SafeNode` is the whole receiver in one object: feed it packets and read output. It is safe to call input and output from separate threads.

```cpp
#include <SimpleSACN.hpp>
using namespace SimpleSACN;

// universe 1, keep the last frame if the source goes away (or DataLossPolicy::STOP to blank)
Node::SafeNode node({1, Node::DataLossPolicy::HOLD});

// network thread: for every UDP packet received
node.onPacket(buf, len, millis());

// output thread: provides a frame of DMX data according to sACN rules for receivers
DmxFrame frame;
if (node.latestFrame(&frame, millis()))
    //frame.data contains the latest output of the node, 512 DMX channels
    doSomething(frame.data);
```

- `millis()` is any millisecond clock; wrap-around safe.
- `latestFrame` returns false when there is nothing to output: no source yet, the source sent terminate, or data loss with `STOP` option set.
- Networking: open a UDP socket on port 5568 and join the multicast group `239.255.<universe high byte>.<universe low byte>` (universe 1 = `239.255.0.1`).

## Other Entry Points

SacnData

> A structure representing an e131 data packet. Byte offsets of items within a data packet are provided as well.

Parse::parse(const uint8_t *buf, size_t len, SacnData &out)

> Attempt to parse a given buffer into a packet of SacnData. Validates the packet against specification for e131 data packets. Handles conversion of network byte ordering into hardware byte ordering.

Core::SacnReceiver

> Configurable on construction and internally applies rules for sACN universe receivers defined in the e131 specification (see Policy namespace). Provides a public API to pass bytes in, and receive DMX frames out. Keeps no internal clock, callers must pass the current time in with input/output requests. Does not provide thread-safety for overlapping input/output calls.

## sACN Implementation

This library implements sACN data packets based on the ANSI E1.31-2025 specification, along with a basic implementation of a single universe receiver.

The parser handles application of e131 data packet rules relating to identification.

The SacnReceiver class utilizes small pure functions in the Policy namespace to further apply rules required of sACN receivers such as data loss behavior, sequence checking of received packets, handling of multiple sources, and packet option bits.

The SacnReceiver class moves through a few states as it is given input. In cases of multiple sources sending on the same universe, it will always move to the highest priority and all channels of DMX will be based on the higher priority source. Sources of equal priority will be selected based on the highest CID. If the higher-priority source stops, a lower-priority source takes over immediately if the higher one sent terminate, or after 2.5 s of data loss if it just went silent.

## Limitations

The SacnReceiver class does not implement Highest-Takes-Precedence on individual channels.

Each instance of the SacnReceiver class will only listen for one universe, and will not respond to sync or discovery packets.

## Running Tests

Library unit tests are plain C++ functions harnessed into CTest. Tests only build when the SimpleSACN project is targeted directly.

```sh
cd SimpleSACN
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

### License

MIT
