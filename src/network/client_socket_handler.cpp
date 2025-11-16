/**
 * @file client_socket_handler.hpp
 * @author Martin Zůbek
 * @date 2025-10-13
 * @brief Implementation of ClientSocketHandler methods. Methods are responsible for handling client socket and resolve the best source address for client.
 */

#include <iostream>
#include <unistd.h>
#include <netdb.h>
#include <stdexcept>
#include <variant>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip_icmp.h>
#include <netinet/icmp6.h>
#include "../../inc/network/client_socket_handler.hpp"
#include "../../inc/types/protocol_definitions.hpp"

ClientSocketHandler::ClientSocketHandler(int type, std::variant<struct sockaddr_in, struct sockaddr_in6> destinationAddress) {
    // Initialize attributes of the class
    this->clientSocketDescriptor = -1;
    this->typeOfAddress = type;
    
    // Resolve which type of socket will be opened
    if(this->typeOfAddress == AF_INET){ // Icmpv4 raw socket
        this->clientSocketDescriptor = socket(this->typeOfAddress, SOCK_RAW, IPPROTO_ICMP);
        // Set option for recive and create own ipv4 header
        int option = 1;
        setsockopt(this->clientSocketDescriptor, IPPROTO_IP, IP_HDRINCL, &option, sizeof(option));
    }
    else if(this->typeOfAddress == AF_INET6){ // Icmpv6 raw socket
        this->clientSocketDescriptor = socket(this->typeOfAddress, SOCK_RAW, IPPROTO_ICMPV6);
    }
    else{ // Invalid type of enemuration
        throw std::runtime_error("Error: Internal problem whith propagating the type of address in the ClientSocketHandler class!");
    }

    // Check if the socket was opened successfully
    if(this->clientSocketDescriptor < 0){
        throw std::runtime_error("Error: Cannot open the socket!");
    }

    // Resolve the source address for program
    this->resolveSourceAddress(destinationAddress);
}

void ClientSocketHandler::resolveSourceAddress(std::variant<struct sockaddr_in, struct sockaddr_in6> destinationAddress) {
  
    // Open the socket for resolving best option for source address
    int resolveSocket = socket(this->typeOfAddress, SOCK_DGRAM, 0);
    if(resolveSocket < 0){
        throw std::runtime_error("Error: Cannot open the socket for resolving source address!");
    }

    int returnValue = 0;

    // Resolve the source address for ipv4
    if(this->typeOfAddress == AF_INET){
        struct sockaddr_in destAddrIpv4 = std::get<struct sockaddr_in>(destinationAddress);
        destAddrIpv4.sin_port = htons(APP_PROTOCOL_PORT);
        // Connect to server for get best option of ipv4 address
        returnValue = connect(resolveSocket, (struct sockaddr*)&destAddrIpv4, sizeof(destAddrIpv4));

        if(returnValue == 0){
            struct sockaddr_in resultAddress;
            socklen_t resultAddrSize = sizeof(resultAddress);
            // Get the best option for ipv4 address from the used socket by connect
            if(getsockname(resolveSocket, (struct sockaddr*)&resultAddress, &resultAddrSize) == 0){
                this->sourceAddress = resultAddress;
            }
        }
    }

    // Resolve the source address for ipv6
    if(this->typeOfAddress == AF_INET6){
        struct sockaddr_in6 destAddrIpv6 = std::get<struct sockaddr_in6>(destinationAddress);
        destAddrIpv6.sin6_port = htons(APP_PROTOCOL_PORT);
        // Connect to server for get best option of ipv6 address
        returnValue = connect(resolveSocket, (struct sockaddr*)&destAddrIpv6, sizeof(destAddrIpv6));
        
        if(returnValue == 0){
            struct sockaddr_in6 resultAddress;
            socklen_t resultAddrSize = sizeof(resultAddress);
            // Get the best option for ipv6 address from the used socket by connect
            if(getsockname(resolveSocket, (struct sockaddr*)&resultAddress, &resultAddrSize) == 0){
                this->sourceAddress = resultAddress;
            }
        }
    }

    close(resolveSocket);
    
    // Catch the error from connect
    if(returnValue != 0){
        throw std::runtime_error("Error: Cannot resolve the source address for the client!");
    }
}

void ClientSocketHandler::closeDescriptors() {
    // Close the client socket
    if(this->clientSocketDescriptor >= 0){
        close(this->clientSocketDescriptor);
        this->clientSocketDescriptor = -1;
    }
}

std::variant<struct sockaddr_in, struct sockaddr_in6> ClientSocketHandler::getSourceAddress() {
    return this->sourceAddress;
}

int ClientSocketHandler::getSocketDescriptor() {
    return this->clientSocketDescriptor;
}