/**
 * @file abstract_socket_handler.hpp
 * @author Martin Zůbek
 * @date 2025-10-13
 * @brief Header file with definition of AbstractSocketHandler class. This  class is an abstract 
 * class for class ClientSocketHandler in the [src|inc]/network/client_socket_handler.[cpp|hpp] 
 * and class ServerSocketHandler in the [src|inc]/network/server_socket_handler.[cpp|hpp].
 * 
 */
#ifndef ABSTRACT_SOCKET_HANDLER_HPP
#define ABSTRACT_SOCKET_HANDLER_HPP

#include <iostream>

/**
 * @class AbstractSocketHandler
 * @brief Abstract class for class ClientSocketHandler in the [src|inc]/network/client_socket_handler.[cpp|hpp]
 * and class ServerSocketHandler in the [src|inc]/network/server_socket_handler.[cpp|hpp] for handling sockets.
 */
class AbstractSocketHandler {
    public:
        /**
         * @brief Virtual destructor.
         */
        virtual ~AbstractSocketHandler() = default;
        /**
         * @brief Virtual method for closing the socket descriptors.
         */
        virtual void closeDescriptors() = 0;
};

#endif // ABSTRACT_SOCKET_HANDLER_HPP