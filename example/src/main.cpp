// Single-threaded SimpleSACN example: listens to universe 1 and prints the first 8 channels.
// For multi-threaded use see jking100/SimpleSACN-ESP32-Node.
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <thread>

#include <asio.hpp>

#include "SimpleSACN.hpp"

int main()
{
    using namespace std::chrono;

    asio::io_context io;
    asio::ip::udp::socket sock(io, asio::ip::udp::v4());
    sock.set_option(asio::ip::udp::socket::reuse_address(true));
    sock.bind({asio::ip::udp::v4(), 5568});                                                  // sACN port
    sock.set_option(asio::ip::multicast::join_group(asio::ip::make_address("239.255.0.1"))); // universe 1
    sock.non_blocking(true);

    SimpleSACN::Node::SafeNode node({1, SimpleSACN::Node::DataLossPolicy::HOLD});
    SimpleSACN::DmxFrame frame;
    std::uint8_t buf[638];
    const auto start = steady_clock::now();

    for (;;)
    {
        auto now = duration_cast<milliseconds>(steady_clock::now() - start).count();

        // drain every packet that arrived since the last pass
        for (;;)
        {
            asio::error_code ec;
            std::size_t len = sock.receive(asio::buffer(buf), 0, ec);
            if (ec == asio::error::would_block)
                break;
            if (ec)
                throw asio::system_error(ec);
            node.onPacket(buf, len, now); // push to the node
        }

        std::printf("\rLive:");
        if (node.latestFrame(&frame, now)) // check the node for frame
            for (int i = 0; i < 8; ++i)
                std::printf(" %3u", frame.data[i]);
        else
            std::printf(" No DMX                         ");
        std::fflush(stdout);

        std::this_thread::sleep_for(milliseconds(20));
    }
}
