#include <cassert>
#include <cstring>
#include <iostream>

#include "Core/SacnReceiver.hpp"

using namespace SimpleSACN;

using Core::SacnReceiver;
using state = SacnReceiver::RxState;

static const SacnReceiver::RxConfig u1_h{1, Policy::TimeoutPolicy::HOLD};
static const SacnReceiver::RxConfig u1_s{1, Policy::TimeoutPolicy::STOP};

// uni 1 packet; cid byte 0, sequence number and channel 1 are the parts the tests vary
static SacnData pkt(uint8_t cid, uint8_t seq, uint8_t ch1 = 255, uint8_t options = 0)
{
    SacnData p{};
    p.universe = 1;
    p.cid[0] = cid;
    p.sequenceNumber = seq;
    p.options = options;
    p.propertyValues[1] = ch1;
    return p;
}

static void test_no_frame_before_packet()
{
    SacnReceiver rx(u1_h);
    DmxFrame out;
    assert(rx.getCurrentState() == state::WAITING);
    assert(!rx.getCurrentFrame(&out, 0));
    std::cout << "test_no_frame_before_packet good\n";
}

// frame holds slots 1-512, start code skipped
static void test_frame_contents()
{
    SacnReceiver rx(u1_h);
    SacnData p = pkt(1, 0);
    p.propertyValues[0] = 9; // start code, must not land in the frame
    p.propertyValues[512] = 42;
    assert(rx.onSacnPacket(p, 0));

    DmxFrame out;
    assert(rx.getCurrentFrame(&out, 0));
    assert(out.data[0] == 255);
    assert(out.data[511] == 42);
    for (int i = 1; i < 511; i++)
        assert(out.data[i] == 0);
    std::cout << "test_frame_contents good\n";
}

static void test_state_transitions()
{
    SacnReceiver rx(u1_h);
    rx.onSacnPacket(pkt(1, 0), 0);
    assert(rx.getCurrentState() == state::STREAMING);

    DmxFrame out;
    rx.getCurrentFrame(&out, 2499);
    assert(rx.getCurrentState() == state::STREAMING);
    rx.getCurrentFrame(&out, 2500);
    assert(rx.getCurrentState() == state::LOST);
    std::cout << "test_state_transitions good\n";
}

static void test_dataloss_stop()
{
    SacnReceiver rx(u1_s);
    rx.onSacnPacket(pkt(1, 0), 0);

    DmxFrame out;
    assert(rx.getCurrentFrame(&out, 2499));
    assert(!rx.getCurrentFrame(&out, 2500));
    std::cout << "test_dataloss_stop good\n";
}

// same priority: greater cid takes over, lesser cid ignored
static void test_cid_arbitration()
{
    SacnReceiver rx(u1_h);
    rx.onSacnPacket(pkt(2, 0), 0);
    assert(!rx.onSacnPacket(pkt(1, 1, 7), 100));

    DmxFrame out;
    assert(rx.getCurrentFrame(&out, 100));
    assert(out.data[0] == 255);

    assert(rx.onSacnPacket(pkt(3, 0, 9), 200));
    assert(rx.getCurrentFrame(&out, 200));
    assert(out.data[0] == 9);
    std::cout << "test_cid_arbitration good\n";
}

// higher priority takes over regardless of cid, lower is ignored
static void test_priority()
{
    SacnReceiver rx(u1_h);
    SacnData a = pkt(5, 0);
    a.priority = 100;
    rx.onSacnPacket(a, 0);

    SacnData lo = pkt(9, 0, 7);
    lo.priority = 99;
    assert(!rx.onSacnPacket(lo, 100));

    SacnData hi = pkt(1, 0, 8);
    hi.priority = 101;
    assert(rx.onSacnPacket(hi, 200));
    DmxFrame out;
    assert(rx.getCurrentFrame(&out, 200));
    assert(out.data[0] == 8);
    std::cout << "test_priority good\n";
}

// a lower cid source takes over once the old source is lost
static void test_cid_takeover()
{
    SacnReceiver rx(u1_h);
    rx.onSacnPacket(pkt(1, 0), 0);

    // loss is detected on input too, without a getCurrentFrame call
    assert(!rx.onSacnPacket(pkt(0, 0, 7), 2499));

    // loses arbitration until the old source is lost
    assert(rx.onSacnPacket(pkt(0, 0, 7), 3000));
    assert(rx.getCurrentState() == state::STREAMING);
    DmxFrame out;
    assert(rx.getCurrentFrame(&out, 3000));
    assert(out.data[0] == 7);
    std::cout << "test_cid_takeover good\n";
}

// terminate drops the stream back to waiting, then any source can start a new one
static void test_terminate()
{
    for (auto cfg : {u1_h, u1_s})
    {
        SacnReceiver rx(cfg);
        rx.onSacnPacket(pkt(1, 0), 0);

        assert(!rx.onSacnPacket(pkt(1, 1, 255, Constants::OPTIONS_STREAM_TERMINATED), 100));
        assert(rx.getCurrentState() == state::WAITING);

        DmxFrame out;
        assert(!rx.getCurrentFrame(&out, 100));

        // new source, old sequence number
        assert(rx.onSacnPacket(pkt(2, 1), 200));
        assert(rx.getCurrentState() == state::STREAMING);
    }
    std::cout << "test_terminate good\n";
}

// terminate from another source while streaming is ignored, even one that would win arbitration
static void test_terminate_other_cid()
{
    SacnReceiver rx(u1_h);
    rx.onSacnPacket(pkt(1, 0), 0);
    assert(!rx.onSacnPacket(pkt(2, 1, 255, Constants::OPTIONS_STREAM_TERMINATED), 100));
    assert(rx.getCurrentState() == state::STREAMING);
    std::cout << "test_terminate_other_cid good\n";
}

// preview data is not displayed and never takes over a stream
static void test_preview()
{
    SacnReceiver rx(u1_h);
    assert(!rx.onSacnPacket(pkt(1, 0, 255, Constants::OPTIONS_PREVIEW_DATA), 0));
    assert(rx.getCurrentState() == state::WAITING);

    rx.onSacnPacket(pkt(1, 1), 100);
    assert(!rx.onSacnPacket(pkt(2, 0, 7, Constants::OPTIONS_PREVIEW_DATA), 200));
    DmxFrame out;
    assert(rx.getCurrentFrame(&out, 200));
    assert(out.data[0] == 255);
    std::cout << "test_preview good\n";
}

int main()
{
    test_no_frame_before_packet();
    test_frame_contents();
    test_state_transitions();
    test_dataloss_stop();
    test_cid_arbitration();
    test_priority();
    test_cid_takeover();
    test_terminate();
    test_terminate_other_cid();
    test_preview();

    std::cout << "All tests passed" << std::endl;
    return 0;
}
