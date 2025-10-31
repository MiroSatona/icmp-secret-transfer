/**
 * @file client_socket_handler.hpp
 * @author Martin Zůbek
 * @date 2025-10-13
 * @brief Header file with definition of ClientSocketHandler class. This  class is responsible for handling sockets in the client program context
 * and resolve the best option for the source address.
 * 
 */
#ifndef CLIENT_SOCKET_HANDLER_HPP
#define CLIENT_SOCKET_HANDLER_HPP

#include <iostream>
#include <variant>
#include "abstract_socket_handler.hpp"

/**
 * @class ClientSocketHandler   
 * @brief Class for handling sockets in the client program context and resolve the best option for the source address.
 */
class ClientSocketHandler : public AbstractSocketHandler {
    private:
        /// @brief The file descriptor of the socket for the client communication.
        int clientSocketDescriptor;
        /// @brief  The type of the address, it will be AF_INET or AF_INET6 from <arpa/inet.h>.
        int typeOfAddress;
        /// @brief The resolved source address.
        std::variant<struct sockaddr_in, struct sockaddr_in6>  sourceAddress;
    public:
        /**
         * @brief Constructor of the ClientSocketHandler class.
         * @param type The type of the address, it will be AF_INET or AF_INET6 from <arpa/inet.h>.
         * @param destinationAddress The variant containing destination address like sockaddr_in structure from <sys/socket.h> or sockaddr_in6 structure from <netinet/in.h>.
         * @throw std::runtime_error If the socket cannot be opened or any internal error will become.
         */
        ClientSocketHandler(int type, std::variant<struct sockaddr_in, struct sockaddr_in6> destinationAddress);

        /**
         * @brief Method for closing the file descriptors of the sockets.
         */
        void closeDescriptors() override;

        /**
         * @brief Method for resolving the best option for the source address by connect function from <sys/socket.h>.
         * 
         * @param destinationAddress Variant containing destination address like sockaddr_in structure from <sys/socket.h> or sockaddr_in6 structure from <netinet/in.h>.
         */
        void resolveSourceAddress(std::variant<struct sockaddr_in, struct sockaddr_in6> destinationAddress);
        /**
         * @brief Getter for the resolved source address.
         * 
         * @return  std::variant<struct sockaddr_in, struct sockaddr_in6> The variant containing resolved source address like sockaddr_in structure from <sys/socket.h> or sockaddr_in6 structure from <netinet/in.h>.
         */
        std::variant<struct sockaddr_in, struct sockaddr_in6> getSourceAddress();
};

#endif // CLIENT_SOCKET_HANDLER_HPP

