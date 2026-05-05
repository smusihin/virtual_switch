#pragma once

#include "IO.h"

#include "EthHeaderView.h"
#include "MACView.h"

class NetworkSwitch
{
public:
    NetworkSwitch(IO& io, size_t portsCount) : m_io{io}, m_portsCount{portsCount}
    {
    }

    void run()
    {
        while (true)
        {
            auto packets = m_io.read();
            if(packets.empty())
            {
                return;
            }

            for (const auto & packet: packets)
            {
                auto decision = ingress(packet);
                egress(m_io, decision, packet.data);
            }
        }
    }

    struct ForwardDecision
    {
        bool drop = false;
        std::vector<size_t> outPorts;
    };

private:
    ForwardDecision ingress(InputPacket inputPacket)
    {
        EthHeaderView eth(inputPacket.data);

        if (!eth.valid())
        {
            return {.drop = true};
        }

        std::cout << "Ethernet type: 0x"
                << std::hex << eth.ethertype() << std::dec << '\n';

        std::cout << "Destination: " << MACView(eth.dst()) << '\n';
        std::cout << "Source: " << MACView(eth.src()) << '\n';

        ForwardDecision fd{};
        for (size_t j = 0; j < m_portsCount; ++j)
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



 private:
    IO& m_io;
    const size_t m_portsCount;
 };