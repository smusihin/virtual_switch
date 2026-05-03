#include "EthHeaderView.h"
#include "MACView.h"

#include <sys/socket.h>
#include <arpa/inet.h> // for htons
#include <net/ethernet.h>
#include <net/if.h>
#include <linux/if_packet.h>
#include <unistd.h>

#include <iostream>
#include <vector>

#include <poll.h>

struct ForwardDecision
{
    bool drop = false;
    std::vector<size_t> outPorts;
};

ForwardDecision ingress(uint8_t* buffer, size_t nread, size_t inPort, size_t portsCount)
{
    EthHeaderView eth(buffer, nread);

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
        if (j != inPort)
            fd.outPorts.push_back(j);
    }

    return fd;
}

void egress(const ForwardDecision& fd,
            uint8_t* buffer,
            size_t nread,
            const std::vector<pollfd>& pollFds)
{
    if (fd.drop) return;

    for (const auto& outPort : fd.outPorts) {
        std::cout << "Write to " << outPort << '\n';
        write(pollFds[outPort].fd, buffer, nread);
    }
}

void runHub(const std::vector<int>& fds) {
    std::vector<pollfd> poll_fds;
    for (int fd : fds) {
        poll_fds.push_back({fd, POLLIN, 0});
    }

    unsigned char buffer[2048];
    struct sockaddr_ll sll;
    while (true) {
        int ret = poll(poll_fds.data(), poll_fds.size(), -1);
        if (ret < 0) break;

        for (size_t i = 0; i < poll_fds.size(); ++i) {
            if (poll_fds[i].revents & POLLIN) {
                socklen_t sll_len = sizeof(sll);

                ssize_t nread = recvfrom(poll_fds[i].fd, buffer, sizeof(buffer), 0,
                                         (struct sockaddr*)&sll, &sll_len);

                if (nread <= 0) continue;

                std::cout << "Received " << nread << " bytes by " << i << '\n';

                if (sll.sll_pkttype == PACKET_OUTGOING) continue;

                auto decision = ingress(buffer, nread, i, poll_fds.size());
                egress(decision, buffer, nread, poll_fds);

            }
        }
    }
}

int createRawSocket(const char* interfaceName)
{
    int sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sock < 0)
    {
        std::cout << "socket error\n";
    }

    struct sockaddr_ll sll;
    sll.sll_family = AF_PACKET;
    sll.sll_ifindex = if_nametoindex(interfaceName);
    sll.sll_protocol = htons(ETH_P_ALL);

    (void)bind(sock, (struct sockaddr *)&sll, sizeof(sll));

    return sock;

}


int main( int argc, char** argv)
{
    std::vector<int> sockets;
    sockets.reserve(argc - 1);
    for (int i = 1; i < argc; ++i)
    {
        sockets.push_back(createRawSocket(argv[i]));
        std:: cout << argv[i] << '\n';
    }

    runHub(sockets);
    return 0;
}
