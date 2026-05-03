#include "EthHeaderView.h"
#include "MACView.h"
#include "IO.h"

#include <iostream>
#include <vector>

struct ForwardDecision
{
    bool drop = false;
    std::vector<size_t> outPorts;
};

ForwardDecision ingress(InputPacket inputPacket, size_t portsCount)
{
    EthHeaderView eth(inputPacket.data.data(), inputPacket.data.size());

    if (!eth.valid())
    {
        return {.drop = true};
    }

    std::cout << "Ethernet type: 0x"
              << std::hex << eth.ethertype() << std::dec << '\n';

    std::cout << "Destination: " << MACView(eth.dst()) << '\n';
    std::cout << "Source: " << MACView(eth.src()) << '\n';

    ForwardDecision fd{};
    for (size_t j = 0; j < portsCount; ++j)
    {
        if (j != inputPacket.inputIdx)
            fd.outPorts.push_back(j);
    }

    return fd;
}

void egress(
    IO& io,
    const ForwardDecision& fd,
    const std::span<uint8_t>& buffer)
{
    if (fd.drop) return;

    for (const auto& outPort : fd.outPorts) {
        std::cout << "Write to " << outPort << '\n';
        io.write(buffer, outPort);
    }
}

int main( int argc, char** argv)
{
    std::vector<std::string_view> interfaces;
    interfaces.reserve(argc - 1);
    for (int i = 1; i < argc; ++i)
    {
        interfaces.push_back(argv[i]);
    }

    IO io(interfaces);

    while (true)
    {
        auto packets = io.read();
        if(packets.empty())
        {
            return 0;
        }
        for (const auto & packet: packets)
        {
            auto decision = ingress(packet, interfaces.size());
            egress(io, decision, packet.data);
        }
    }

    return 0;
}
