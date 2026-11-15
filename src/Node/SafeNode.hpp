#pragma once

#include <cstdint>
#include <cstddef>
#include <mutex>

#include "../E131/SacnData.hpp"
#include "../E131/SacnParse.hpp"
#include "../E131/DmxFrame.hpp"

#include "../Core/SacnReceiver.hpp"

namespace SimpleSACN
{

namespace Node
{
    using Config = Core::SacnReceiver::RxConfig;
    using DataLossPolicy = Policy::TimeoutPolicy;

    // single universe sacnNode with configurable data loss behavior, and a basic in/out api
    // given raw bytes, this node will process and serve frames of dmx data according to sacn operation policy
    // its input and output functions are thread safe, output/imput may wait on a copy of SacnData + accoutrements
    class SafeNode
    {
    public:
        SafeNode(Config c) : _rx(c) {}

        bool onPacket(const uint8_t *buf, size_t len, uint32_t timeNowMs);

        bool latestFrame(DmxFrame *out, uint32_t timeNowMs);

    private:
        std::mutex _guard{};
        Core::SacnReceiver _rx;
    };

    inline bool SafeNode::onPacket(const uint8_t *buf, size_t len, uint32_t timeNowMs)
    {
        SacnData tmp;
        if (!Parse::parse(buf, len, tmp))
            return false;

        const std::lock_guard<std::mutex> lock(_guard);

        return _rx.onSacnPacket(tmp, timeNowMs);
    }

    inline bool SafeNode::latestFrame(DmxFrame *out, uint32_t timeNowMs)
    {
        const std::lock_guard<std::mutex> lock(_guard);

        return _rx.getCurrentFrame(out, timeNowMs);
    }
}
} // namespace SimpleSACN
