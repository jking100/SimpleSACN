#pragma once

#include <cstdint>

namespace SimpleSACN
{

/*
Representation of a full dmx universe
*/
struct DmxFrame
{
    // first byte is 0x00 always, 512 channels of 0-255 remain
    uint8_t data[512]{};
};
} // namespace SimpleSACN
