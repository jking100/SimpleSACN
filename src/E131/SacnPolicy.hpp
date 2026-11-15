#pragma once
#include <cstdint>
#include <cstring>

#include "SacnData.hpp"

namespace SimpleSACN
{

namespace Policy
{
    enum class TimeoutPolicy
    {
        HOLD,
        STOP
    };

    // 2500ms or longer is defined as data loss by e131 6.7.1
    // signed diff: a now slightly before rx (clock read before the lock by another thread) is not loss
    constexpr bool dataLoss(uint32_t nowMs, uint32_t rxMs) { return int32_t(nowMs - rxMs) >= 2500; }

    // reject the same packet or packets up to 19 behind, handle wrap
    inline bool outOfOrder(uint8_t seqNow, uint8_t seqPrior)
    {
        int8_t diff = static_cast<int8_t>(seqNow - seqPrior);
        return (diff <= 0 && diff > -20);
    }

    inline bool sameSource(const SacnData &a, const SacnData &b) { return memcmp(a.cid, b.cid, sizeof(a.cid)) == 0; }

    // relevant if right universe and, once started: same source in order, or a higher priority source,
    // or same priority with a greater cid. cid tiebreak is deterministic, unlike first-come which depends on network jitter
    inline bool relevantPacket(const SacnData &p, uint16_t universe, bool alrStarted, const SacnData &last)
    {
        if (p.universe != universe)
            return false;
        if (!alrStarted)
            return true;

        int cidCmp = memcmp(p.cid, last.cid, sizeof(p.cid));
        if (cidCmp == 0)
            return !outOfOrder(p.sequenceNumber, last.sequenceNumber); // sequence is per source
        if (p.priority != last.priority)
            return p.priority > last.priority;
        return cidCmp > 0;
    }

    inline bool shouldTerminate(const SacnData &p)
    {
        return p.isTerminated();
    }

    inline bool shouldNotDisplay(const SacnData &p)
    {
        return p.isPreview();
    }

    inline bool shouldServeFrames(TimeoutPolicy tP, uint32_t nowMs, uint32_t lastPacketMs)
    {
        return tP == TimeoutPolicy::HOLD || !dataLoss(nowMs, lastPacketMs);
    }
}
} // namespace SimpleSACN
