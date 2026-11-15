
# SimpleSACN

*A header-only library implementing the ANSI E1.31 sACN standard, allowing for the conversion of sACN network packets into DMX frames following the sACN standard, and user defined behavior options*

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

## Running Tests

Library unit tests are plain C++ functions harnessed into CTest. Tests only build when the SimpleSACN project is targeted directly.

```sh
cd .../SimpleSACN/
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```
