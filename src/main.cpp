
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <iostream>
#include <string>
#include <vector>

int main(int argc, char **argv)
{
    std::string addr = "::";
    int port = 12345;

    if(argc >= 3)
    {
        for(int i = 0; i < argc; i++)
        {
            std::string arg = argv[i];
            int next = i + 1;

            if(arg == "--port" && next < argc)
                port = std::stoi(argv[next]);
            else if(arg == "--addr" && next < argc)
                addr = argv[next];
        }
    }

    int udpServer;

    if((udpServer = socket(AF_INET6, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket");
        return -1;
    }

    int onlyIpv6 = 0;

    if(setsockopt(udpServer, IPPROTO_IPV6, IPV6_V6ONLY, &onlyIpv6, sizeof(onlyIpv6)) < 0)
    {
        perror("setsockopt");
        close(udpServer);
        return -1;
    }

    sockaddr_in6 serverAddr {};
    serverAddr.sin6_family = AF_INET6;
    serverAddr.sin6_port = htons(port);

    if(addr.find(":") != std::string::npos)
    {
        if(inet_pton(AF_INET6, addr.c_str(), &serverAddr.sin6_addr) != 1)
        {
            perror("inet_pton");
            close(udpServer);
            return -1;
        }
    }
    else
    {
        std::string mapped = "::ffff:" + addr;

        if(inet_pton(AF_INET6, mapped.c_str(), &serverAddr.sin6_addr) != 1)
        {
            perror("inet_pton");
            close(udpServer);
            return -1;
        }
    }

    if(bind(udpServer, (sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        perror("bind");
        close(udpServer);
        return -1;
    }

    std::vector<char> buffer(1024);

    while(true)
    {
        sockaddr_in6 clientAddr {};
        socklen_t clientLen = sizeof(clientAddr);

        ssize_t bytes = recvfrom(udpServer, buffer.data(), buffer.size(), 0, (sockaddr *)&clientAddr, &clientLen);

        if(bytes > 0)
        {
            std::cout << "Echo: " << bytes << " bytes\n";

            ssize_t sent = sendto(udpServer, buffer.data(), static_cast<size_t>(bytes), 0, (sockaddr *)&clientAddr, clientLen);

            if(sent < 0)
            {
                perror("sendto");
            }
        }
        else if(bytes < 0)
        {
            perror("recvfrom");
        }
    }

    close(udpServer);

    return 0;
}
