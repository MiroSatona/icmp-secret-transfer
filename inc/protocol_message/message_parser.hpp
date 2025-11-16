/**
 * @file message_parser.hpp
 * @author Martin Zůbek
 * @date 2025-10-20
 * @brief Header file with definition of MessageParser class. This class is derived from MessageHandler class in [src|inc]/protocol_message.
 *        This class contains the methods for parsing messages of the protocol.
 */
#ifndef MESSAGE_PARSER_HPP
#define MESSAGE_PARSER_HPP

#include <iostream>
#include <variant>
#include <netinet/in.h>
#include <netinet/ip.h> 
#include "../types/struct_cipher.hpp"
#include "../types/protocol_definitions.hpp"
#include "../../inc/protocol_message/message_handler.hpp"

/**
 * @class MessageParser
 * @brief Class for parsing messages of the protocol.
 */
class MessageParser : public MessageHandler{
    private:
        /**
         * @brief Method for validate Ipv4 header.
         * 
         * @param packet Packet with Ipv4 header.
         * @param packetSize Size of the packet.
         * @return @return bool True if the IPv4 header is valid, otherwise false.
         */
        bool isIpv4HeaderValid(unsigned char* packet, size_t packetSize);
        /**
         * @brief Method for validate ICMP header.
         * 
         * @param packet Packet with ICMP header.
         * @param packetSize Size of the packet.
         * @param id Identifier to match in the ICMP header.
         * @param seqNumber Sequence number to match in the ICMP header.
         * @return bool True if the ICMP header is valid, otherwise false.
         */
        bool isIcmpHeaderValid(unsigned char* packet, size_t packetSize,  uint16_t id, uint16_t seqNumber);
    public:
        /**
         * @brief Constructor of MessageParser class
         */
        MessageParser();
        /**
         * @brief Method for parsing message of the protocol. 
         * 
         * @param packet Packet which will be parsed. 
         * @param packetSize Size of the parsed packet.
         * @param id Identifier to match in the ICMP header.
         * @param seqNumber Sequence number to match in the ICMP header
         * @param outCipherData Reference on struct CHIPHER_T from inc/types/struct_cipher.hpp where will be stord chipher data.
         * @return MESSAGE_TYPES from inc/protocol_message/message_handler.hpp represents messages of the protocol.
         */
        MESSAGE_TYPES parseMessage(unsigned char* packet, size_t packetSize, uint16_t id, uint16_t seqNumber, struct CHIPHER_T &outCipherData);
};

#endif // MESSAGE_PARSER_HPP