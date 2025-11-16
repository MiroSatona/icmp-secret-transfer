/**
 * @file message_creator.hpp
 * @author Martin Zůbek
 * @date 2025-10-20
 * @brief Header file with definition of MessageCreator class. This class is derived from MessageHandler class in [src|inc]/protocol_message.
 *        This class contains the methods for creating messages of the protocol.
 */
#ifndef MESSAGE_CREATOR_HPP
#define MESSAGE_CREATOR_HPP

#include <iostream>
#include <variant>
#include <netinet/in.h>
#include "message_handler.hpp"
#include "../types/struct_cipher.hpp"
#include "../types/protocol_definitions.hpp"

/**
 * @class MessageCreator
 * @brief Class for creating messages of the protocol.
 */
class MessageCreator : public MessageHandler {
    public:
        /**
         * @brief Constructor of the MessageCreator class.
         * 
         * @param type Type of Ip version (Ipv4 or Ipv6) represented by AF_INET or AF_INET6 from <arpa/inet.h>.
         * @param destAddress Destination address (IPv4 or IPv6) represented by sockaddr_in or sockaddr_in6 from <netinet/in.h>.
         * @param srcAddress Source address (IPv4 or IPv6) represented by sockaddr_in or sockaddr_in6 from <netinet/in.h>.
         */
        MessageCreator(int type, 
                       std::variant<struct sockaddr_in, struct sockaddr_in6> destAddress,
                       std::variant<struct sockaddr_in, struct sockaddr_in6> srcAddress);
        /**
         * @brief Method to create message of the protocol with data.
         * 
         * @param dataToSend Data in struct CHIPHER_T from inc/types/struct_cipher.hpp.
         * @param id Identifier of ICMP message.
         * @param seqNumber Sequence number of ICMP message.
         * @param msgType Type of protocol message from inc/types/protocol_definitions.hpp.
         * @param outSize Reference where will be stored size of the created message.
         * @param icmpType Type of ICMP message for IPv4.
         * @param icmp6Type Type of ICMP message for IPv6.
         * @return unsigned char* Created message of the protocol with encrypted data.
         */
        unsigned char * createMessageWithData(CHIPHER_T dataToSend, uint16_t id, uint16_t seqNumber, MESSAGE_TYPES msgType, size_t& outSize, uint8_t icmpType, uint8_t icmp6Type);
        /**
         * @brief Create message without data
         * 
         * @param dataToSend Data in struct CHIPHER_T from inc/types/struct_cipher.hpp.
         * @param id Identifier of ICMP message.
         * @param seqNumber Sequence number of ICMP message.
         * @param msgType Type of protocol message from inc/types/protocol_definitions.hpp.
         * @param outSize Reference where will be stored size of the created message.
         * @param icmpType Type of ICMP message for IPv4.
         * @param icmp6Type Type of ICMP message for IPv6.
         * @return unsigned char* Created message of the protocol with encrypted data.
         */
        unsigned char * createMessageWithNoData(uint16_t id, uint16_t seqNumber, MESSAGE_TYPES msgType, size_t& outSize, uint8_t icmpType, uint8_t icmp6Type);
};

#endif // MESSAGE_CREATOR_HPP