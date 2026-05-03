#include <sys/types.h> // see `man socket`
#include <sys/socket.h>
#include <arpa/inet.h> // for htons
#include <net/ethernet.h>
#include <net/if.h>
#include <linux/if_packet.h>
#include <unistd.h>

#include <iostream>
#include <vector>

#include <poll.h>

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

                for (size_t j = 0; j < poll_fds.size(); ++j) {
                    if (i != j) {
                        std::cout << "Write to " << j << '\n';
                        write(poll_fds[j].fd, buffer, nread);
                    }
                }
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
