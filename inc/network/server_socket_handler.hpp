/**
 * @file server_socket_handler.hpp
 * @author Martin Zůbek
 * @date 2025-10-13
 * @brief Header file with definition of ServerSocketHandler class. This  class is responsible for handling sockets in the server program context.
 *        Also is responsible for masking the SIGINT signal on the file descriptor.
 */
#ifndef SERVER_SOCKET_HANDLER_HPP
#define SERVER_SOCKET_HANDLER_HPP

#include <poll.h>
#include <vector>
#include <netinet/in.h>
#include <iostream>
#include "abstract_socket_handler.hpp"

// Constant represents index of IPv4 socket descriptor in pollFileDescriptors array
#define IPV4_DESCRIPTOR 0
// Constant represents index of SIGINT signal descriptor in pollFileDescriptors array
#define SIGINT_DESCRIPTOR 1
// Constant represents default size of poll file descriptors -> IPv4 socket + SIGINT signal
#define DEFAULT_SIZE_OF_POLL_FDS 2

/**
 * @class ServerSocketHandler   
 * @brief Class for handling sockets in the server program context and handle SIGINT singal on the file file descriptor.
 */
class ServerSocketHandler : public AbstractSocketHandler {
    private:
        /// @brief Array of struct pollfd from poll.h for store file descriptors of opened sockets and SIGINT singal for poll.
        struct pollfd *pollFileDescriptors;
        /// @brief Count of poll file descriptors.
        int totalPollFileDescriptors;
        /**
         * @brief Method for openning Icmpv4 raw socket for server.
         * @throw std::runtime_error If the socket cannot be opened or set socket options fails.
         */
        void openIpv4Socket();
        /**
         * @brief Method for openning Icmpv6 raw sockets for all available IPv6 interfaces. 
         * @param ipv6Interfaces Vector of struct sockaddr_in6 from netinet/in.h representing active IPv6 interfaces of the server.
         * @throw std::runtime_error If the socket cannot be opened or bind the socket to the actvive IPv6 interface fails.
         */
        void openIpv6Sockets(std::vector<struct sockaddr_in6> ipv6Interfaces);
        /**
         * @brief Method for masking SIGINT signal for handling it on the file descriptor for poll.
         * @throw std::runtime_error If blocking the SIGINT signal or creating the signal file descriptor fails.
         */
        void maskSigintSignal();
        /**
         * @brief Method for getting all active IPv6 interfaces of the server.
         * 
         * @param outSize Pointer on the integer, wich represents the size of returned vector of IPv6 interfaces.
         * @return struct std::vector<struct sockaddr_in6> 
         * @throw std::runtime_error If getifaddrs from <ifaddrs.h> fails.
         */
        struct std::vector<struct sockaddr_in6> getInterfaceForIpv6(int *outSize);
    public:
        /**
         * @brief Construct of the ServerSocketHandler class.
         */
        ServerSocketHandler();
        /**
         * @brief Method for closing all opened file descriptors and deallocating memory for array of poll file descriptors.
         */
        void closeDescriptors() override;
        /**
         * @brief Getter for array of poll file descriptors.
         * @return struct pollfd* Array of struct pollfd from poll.h representing all opened sockets and SIGINT signal for poll.
         */
        struct pollfd* getAllPollFileDescriptors();
        /**
         * @brief Getter for count of all opened file descriptors.
         * @return int Count of all opened file descriptors.
         */
        int getTotalPollFileDescriptors();
};

#endif // SERVER_SOCKET_HANDLER_HPP

