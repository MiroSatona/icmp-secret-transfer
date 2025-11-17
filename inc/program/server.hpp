/** 
 * @file server.hpp
 * @author Martin Zůbek
 * @date 2025-10-25
 * @brief Header file for Server class. There are implemented methods for server-side operations.
 */

#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <string>
#include <memory>
#include <sys/socket.h>
#include <netinet/in.h>
#include <variant>
#include <cstdint>
#include "abstract_program.hpp"
#include "../cipher/decryptor.hpp"
#include "../file_handle/file_writer.hpp"
#include "../types/protocol_definitions.hpp"
#include "../network/server_socket_handler.hpp"
#include "../protocol_message/message_parser.hpp"
#include "../protocol_message/message_creator.hpp"


/**
 * @brief Class representing the server-side program.
 */
class Server : public AbstractProgram{
    private:
        /// @brief Current state of the server
        SERVER_STATE state;
        /// @brief Server address
        std::variant<struct sockaddr_in, struct sockaddr_in6> clientAddress;
        /// @brief Decryptor instance for decrypting data
        std::unique_ptr<Decryptor> decryptor;
        /// @brief Server socket handler for handle sockets
        std::unique_ptr<ServerSocketHandler> serverSocketHandler;
        /// @brief File writer for writing output file
        std::unique_ptr<FileWriter> fileWriter;
        /// @brief Message creator for create protocol messages
        std::unique_ptr<MessageCreator> messageCreator;
        /// @brief Message parser for parse protocol messages
        std::unique_ptr<MessageParser> messageParser;

        /// @brief IP version type (IPv4 or IPv6) of the client represented by AF_INET or AF_INET6 from <arpa/inet.h>.
        int clientIpvType;
        /// @brief Socket descriptor for communication with the client.
        int clientSocketDescriptor;
        /// @brief Actual sequence number for communication with the client.
        uint16_t actualSequenceNumber;
        /// @brief Actual identifier for communication with the client.
        uint16_t actualIdentifier;

  
        /**
         * @brief Method for handling listening state of the server, for waiting new client connections.
         */
        void handleListeningState();
        /**
         * @brief Method for handling receiving state of the server, for receiving data from the client, decrypting it and writing to file.
         * 
         */
        void handleReceivingState();
        /**
         * @brief Method for handling ending state of the server, 
         * where the session with the client is correctly terminated, server sends to the client FIN_ACK message for acknowledging the termination from client side.
         * @throw std::runtime_error If sendto, recvfrom, poll fails or writing to file fails.
         * 
         */
        void handleEndingState();
        /**
         * @brief Method for handling initializing state of the server, for initializing the informations about the client and preparing to receive data.
         * @throw std::runtime_error If sendto, recvfrom, getsockname, poll fails.
         */
        void handleInitializingState();
        /**
         * @brief Method for handling terminate session state of the server, for terminating the session with the non-active client.
         * @throw std::runtime_error If sendto, recvfrom, poll fails.
         */
        void handleTerminateSession();
        /**
         * @brief Method for sending message to the client.
         * @param msgType Type of message of the protocol to send to the client.
         * @throw std::runtime_error If invalid message type is provided.
         * @return ssize_t Number of sent bytes or -1 if sending fails.
         */
        ssize_t sendMessageToClient(MESSAGE_TYPES msgType);
        
    public:
        /**
         * @brief Constructor of the Server class.
         */
        Server();
        /**
         * @brief Method to run the server program and handle states logic of the server.
         */
        void run() override;
};

#endif // SERVER_HPP