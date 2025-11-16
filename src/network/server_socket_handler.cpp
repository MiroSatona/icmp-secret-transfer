/**
 * @file server_socket_handler.cpp
 * @author Martin Zůbek
 * @date 2025-10-13
 * @brief Implementation file of ServerSocketHandler class. This  class is responsible for handling sockets in the server program context
 *        and for masking the SIGINT signal on the file descriptor.
 */
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>
#include <csignal>
#include <sys/signalfd.h>
#include <sys/epoll.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <cstring>
#include <cerrno>
#include <vector>
#include "../../inc/network/server_socket_handler.hpp"

ServerSocketHandler::ServerSocketHandler(){
    // Get all active IPv6 interfaces
    int outSizeOfIpv6Interfaces = 0;
    std::vector<struct sockaddr_in6> ipv6Interfaces = this->getInterfaceForIpv6(&outSizeOfIpv6Interfaces);
    // Allocate poll file descriptors array with known size
    this->totalPollFileDescriptors = DEFAULT_SIZE_OF_POLL_FDS + outSizeOfIpv6Interfaces;
    this->pollFileDescriptors = new struct pollfd[this->totalPollFileDescriptors];

    // Initialize all poll file descriptors
    for(int i = 0; i < outSizeOfIpv6Interfaces; i++){
        this->pollFileDescriptors[i].fd = -1;
        this->pollFileDescriptors[i].events = 0;
        this->pollFileDescriptors[i].revents = 0;
    }

    // Open Ipv4 socket, mask SIGINT signal and open IPv6 sockets
    this->openIpv4Socket();
    this->maskSigintSignal();
    this->openIpv6Sockets(ipv6Interfaces);
}

std::vector<struct sockaddr_in6> ServerSocketHandler::getInterfaceForIpv6(int *outSize){
    // Get all interfaces of the server
    struct ifaddrs *interfaces = nullptr;
    if(getifaddrs(&interfaces) == -1){
        throw std::runtime_error("Error: getifaddrs failed!");
    }

    // Vector for storing active IPv6 interfaces
    std::vector<struct sockaddr_in6> ipv6Addresses;
    for(struct ifaddrs *interface = interfaces; interface != nullptr; interface = interface->ifa_next){
        if(interface->ifa_addr == nullptr) continue;
        // Filter not active IPv6 interfaces and not active interfaces and multicast addresses
        bool filterCondtion = interface->ifa_addr->sa_family == AF_INET6 && (interface->ifa_flags & IFF_UP) && !IN6_IS_ADDR_MULTICAST(&((struct sockaddr_in6*)interface->ifa_addr)->sin6_addr);
        // Save the filtered interefaces to vector form linked list of interfaces
        if(filterCondtion){
            struct sockaddr_in6* address = (struct sockaddr_in6*)interface->ifa_addr;
            ipv6Addresses.push_back(*address);
        }
    }

    // Save the size of the stored IPv6 interfaces, free memory and return interfaces
    *outSize = ipv6Addresses.size();
    freeifaddrs(interfaces);
    return ipv6Addresses;
}

void ServerSocketHandler::openIpv4Socket() {
    // Open raw socket for Icmpv4
    int socketDescriptor = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if(socketDescriptor < 0){
        throw std::runtime_error("Error: Open of IPv4 socket failed!");
    }

    // Set socket options for create own Ipv4 header and get the header in received packets
    int optionValue = 1;
    if(setsockopt(socketDescriptor, IPPROTO_IP, IP_HDRINCL, &optionValue, sizeof(optionValue)) < 0){
        throw std::runtime_error("Error: Setting socket options for IPv4 socket failed!");
    }

    // Save the opnened socket descriptor
    this->pollFileDescriptors[IPV4_DESCRIPTOR].fd = socketDescriptor;
    this->pollFileDescriptors[IPV4_DESCRIPTOR].events = POLLIN;
}

void ServerSocketHandler::openIpv6Sockets(std::vector<struct sockaddr_in6> ipv6Interfaces) {
    // Open raw sockets for Icmpv6 for all active IPv6 interfaces
    for(int indexOfPollFd = DEFAULT_SIZE_OF_POLL_FDS; indexOfPollFd < this->totalPollFileDescriptors; indexOfPollFd++){
        // Open raw socket for Icmpv6
        int socketDescriptor = socket(AF_INET6, SOCK_RAW, IPPROTO_ICMPV6);
        if(socketDescriptor < 0){
            throw std::runtime_error("Error: Open of IPv6 socket failed!");
        }

        // Get the IPv6 interface from vector
        int interfaceIndex = indexOfPollFd - DEFAULT_SIZE_OF_POLL_FDS;
        struct sockaddr_in6 ipv6Address = ipv6Interfaces[interfaceIndex];
        
        // Bind the socket to the active IPv6 interface
        if (bind(socketDescriptor, (struct sockaddr*)&ipv6Address,sizeof(ipv6Address)) < 0) {
            this->closeDescriptors();
            throw std::runtime_error("Error: Bind for IPv6 socket failed!");
        }
        
        // Store the opened and binded socket descriptor
        this->pollFileDescriptors[indexOfPollFd].fd = socketDescriptor;
        this->pollFileDescriptors[indexOfPollFd].events = POLLIN;
    }
}

/**
 * @note The following implmentation was taken from my previous project.
 * 
 * References:
 *  Zůbek, M. Second Project for the IPK Course (IPK25-CHAT Client).
 *  Brno: FIT BUT, 2025. Unpublished software, private GitHub repository.
 */
void ServerSocketHandler::maskSigintSignal(){
    // Set of signals
    sigset_t setSignal;
    // Initialize set
    sigemptyset(&setSignal);
    // Add SIGINT signal to set
    sigaddset(&setSignal, SIGINT);
    // Add SIGTERM signal to set
    sigaddset(&setSignal, SIGTERM);

    // Block normal functionality of SIGINT and SIGTERM signals and set them to be handled by file descriptor
    if (sigprocmask(SIG_BLOCK, &setSignal, NULL) == -1) {
        this->closeDescriptors();
        throw std::runtime_error("Failed to block SIGINT (Ctrl+C) and SIGTERM signals!\n");
    }
    // Create file descriptor and check if it was created successfully
    int signalDescriptor = signalfd(-1, &setSignal, SFD_NONBLOCK);
    if (signalDescriptor == -1) {
        this->closeDescriptors();
        throw std::runtime_error("Failed to create signal file descriptor!\n");
    }

    // Save the created signal file descriptor
    this->pollFileDescriptors[SIGINT_DESCRIPTOR].fd = signalDescriptor;
    this->pollFileDescriptors[SIGINT_DESCRIPTOR].events = POLLIN;
}


void ServerSocketHandler::closeDescriptors(){
    // Close all opened file descriptors
    for(int indexOfDescriptor = 0; indexOfDescriptor < this->totalPollFileDescriptors; indexOfDescriptor++){
        if(this->pollFileDescriptors[indexOfDescriptor].fd != -1){
            close(this->pollFileDescriptors[indexOfDescriptor].fd);
        }
    }
    // Deallocate memory for poll file descriptors array
    delete[] this->pollFileDescriptors;
}

struct pollfd* ServerSocketHandler::getAllPollFileDescriptors(){
    return this->pollFileDescriptors;
}

int ServerSocketHandler::getTotalPollFileDescriptors(){
    return this->totalPollFileDescriptors;
}