/** 
 * @file client.hpp
 * @author Martin Zůbek
 * @date 2025-10-25
 * @brief Header file with declaration of Client class. This class is derived from AbstractProgram class in [src|inc]/program.
 *        This context of program is used for client-side operations.
 */

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <iostream>
#include <string>
#include <memory>
#include <sys/socket.h>
#include <netinet/in.h>
#include <variant>
#include <cstdint>
#include "abstract_program.hpp"
#include "../cipher/encryptor.hpp"
#include "../file_handle/file_reader.hpp"
#include "../types/protocol_definitions.hpp"
#include "../network/client_socket_handler.hpp"
#include "../protocol_message/message_parser.hpp"
#include "../protocol_message/message_creator.hpp"

/**
 * @class Client
 * @brief Class for client-side operations of the program. 
 */
class Client : public AbstractProgram{
    private:
        /// @brief Current state of the client
        CLIENT_STATE state;
        /// @brief Server address
        std::variant<struct sockaddr_in, struct sockaddr_in6> serverAddress;
        /// @brief Encryptor instance for encrypting data
        std::unique_ptr<Encryptor> encryptor;
        /// @brief Client socket handler for handle sockets
        std::unique_ptr<ClientSocketHandler> clientSocketHandler;
        /// @brief File reader for reading input file
        std::unique_ptr<FileReader> fileReader;
        /// @brief Message creator for create protocol messages
        std::unique_ptr<MessageCreator> messageCreator;
        /// @brief Message parser for parse protocol messages
        std::unique_ptr<MessageParser> messageParser;

        /// @brief Count of the authentication retries
        int retryAuth;
        /// @brief Actual sequence number for ICMP messages
        uint16_t sequenceNumber;
        /// @brief Identifier for of client for ICMP messages
        uint16_t identifier;

        /**
         * @brief Method to handle the authentication state of the client.
         * @throw std::runtime_error If sendto, recvfrom, poll fials or any internal error will become.
         */
        void handleAuthState();
        /**
         * @brief Method to handle the wait state of the client.
         * @throw std::runtime_error If max retries of authentication reached.
         */
        void handleWaitState();
        /**
         * @brief Method to handle the sending state of the client.
         * @throw std::runtime_error If sendto, recvfrom, poll fials or any internal error will become.
         */
        void handleSendingState();
        /**
         * @brief Method to handle the finish state of the client.
         * @throw std::runtime_error If sendto, recvfrom, poll fials or any internal error will become.
         */
        void handleFinState();
        /**
         * @brief Method for receiving message from server.
         * 
         * @param packet Pointer to the packet where will be stored received message.
         * @param packetSize Reference where will be stored size of the received packet.
         * @param receivedData Reference on struct CHIPHER_T from inc/types/struct_cipher.hpp where will be stored chipher data.
         * @return MESSAGE_TYPES Type of the received message of the protocol, from inc/types/protocol_definitions.hpp.
         * @throw std::runtime_error If recvfrom fails.
         */
        MESSAGE_TYPES receiveMessageFromServer(unsigned char* packet, size_t& packetSize, CHIPHER_T& receivedData);
    public:
        /**
         * @brief Constructor of Client class.
         * 
         * @param serverAddress Server address represented by sockaddr_in or sockaddr_in6 from <netinet/in.h>.
         * @param filePath Path to the input file to be sent.
         * @param type Type of Ip version (Ipv4 or Ipv6) represented by AF_INET or AF_INET6 from <arpa/inet.h>.
         * @throw std::invalid_argument If the file path is invalid.
         */
        Client(std::variant<struct sockaddr_in, struct sockaddr_in6> serverAddress, std::string filePath, int type);
        /**
         * @brief Method to run the client program and handle states of the client.
         * @throw std::runtime_error If any internal error will become.
         * @throw std::invalid_argument If the file path is invalid.
         * @throw std::logic_error If protocol error from server will become.
         */
        void run() override;
};

#endif // CLIENT_HPP
