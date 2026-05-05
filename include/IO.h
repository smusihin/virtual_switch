#pragma once

#include <cstdint>
#include <vector>
#include <span>
#include <string_view>
#include <iostream>

#include <sys/socket.h>
#include <arpa/inet.h> // for htons
#include <net/ethernet.h>
#include <net/if.h>
#include <linux/if_packet.h>
#include <unistd.h>
#include <poll.h>

struct InputPacket
{
    size_t inputIdx;
    std::span<uint8_t> data;
};

class IO
{
    constexpr static size_t kInputBufferSize = 2048;
public:
    IO(const std::vector<std::string_view>& interfaces)
    {
        m_pollFds.reserve(interfaces.size());
        m_inputBuffers.resize(interfaces.size());
        for (const auto &interface: interfaces)
        {
            m_pollFds.push_back({createRawSocket(interface),POLLIN, 0});
        }
    }

    std::vector<InputPacket> read()
    {
        int ret = poll(m_pollFds.data(), m_pollFds.size(), -1);
        if (ret < 0) 
            return {};

        std::vector<InputPacket> result;
        struct sockaddr_ll sll;
        for (size_t i = 0; i < m_pollFds.size(); ++i) {
            if (m_pollFds[i].revents & POLLIN) {
                socklen_t sll_len = sizeof(sll);

                auto *data = m_inputBuffers[i].data();

                ssize_t nread = recvfrom(m_pollFds[i].fd, data, kInputBufferSize, 0,
                                         (struct sockaddr*)&sll, &sll_len);

                if (nread <= 0) continue;

                result.push_back(InputPacket{i, std::span<uint8_t>(data, nread)});
            }
        }

        return result;
    }

    void write(const std::span<uint8_t>& data, size_t outputIndex)
    {
        ::write(m_pollFds[outputIndex].fd, data.data(), data.size());
    }

private:
    int createRawSocket(std::string_view interface)
    {
        int sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
        if (sock < 0)
        {
            std::cout << "socket error\n";
        }

        struct sockaddr_ll sll;
        sll.sll_family = AF_PACKET;
        sll.sll_ifindex = if_nametoindex(interface.data());
        sll.sll_protocol = htons(ETH_P_ALL);

        (void)bind(sock, (struct sockaddr *)&sll, sizeof(sll));

        return sock;
    }

private:
    std::vector<pollfd> m_pollFds;
    std::vector<std::array<uint8_t, kInputBufferSize>> m_inputBuffers;
};
