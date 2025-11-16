/**
 * @file message_parser.cpp
 * @author Martin Zůbek
 * @date 2025-10-20
 * @brief Implementation of methods of MessageParser class. There are methods for parsing and validating messages of the protocol.
 */
#include <iostream>
#include <stdexcept>
#include <cstring>
#include <variant>
#include <arpa/inet.h>
#include <netinet/icmp6.h>
#include <netinet/ip_icmp.h>
#include "../../inc/types/struct_cipher.hpp"
#include "../../inc/types/protocol_definitions.hpp"
#include "../../inc/protocol_message/message_handler.hpp"
#include "../../inc/protocol_message/message_parser.hpp"


// Constructor of MessageParser class
MessageParser::MessageParser()
    : MessageHandler(
        AF_UNSPEC,
        std::variant<struct sockaddr_in, struct sockaddr_in6>{sockaddr_in{}},
        std::variant<struct sockaddr_in, struct sockaddr_in6>{sockaddr_in{}}
){}

bool MessageParser::isIpv4HeaderValid(unsigned char* packet, size_t packetSize) {

    // Check the size of the packet
    if(packetSize < IPV4_HEADER_SIZE) return false;
    
    // Copy the IPv4 header from the packet
    struct iphdr ipHeader;
    memset(&ipHeader, 0, sizeof(struct iphdr));
    memcpy(&ipHeader, packet, sizeof(struct iphdr));
    
    // Validate the fields of the IPv4 header
    if(ipHeader.version != 4) return false;
    if(ipHeader.ihl < 5) return false;
    if(ipHeader.protocol != IPPROTO_ICMP) return false;
    
    // Calculate and verify the checksum of the IPv4 header
    struct iphdr ipHeaderTmp;
    memcpy(&ipHeaderTmp, &ipHeader, sizeof(struct iphdr));
    ipHeaderTmp.check = 0;
    unsigned short calculatedChecksum = this->calculateChecksum((unsigned char*)&ipHeaderTmp, IPV4_HEADER_SIZE, false);
    if(ipHeader.check != calculatedChecksum) return false;
    
    // Validate source and destination IP addresses
    struct sockaddr_in destAddr = std::get<struct sockaddr_in>(this->destAddress);
    struct sockaddr_in srcAddr = std::get<struct sockaddr_in>(this->srcAddress);
    if(memcmp(&ipHeader.daddr, &destAddr.sin_addr.s_addr, sizeof(uint32_t)) || memcmp(&ipHeader.saddr, &srcAddr.sin_addr.s_addr, sizeof(uint32_t))) {
        return false;
    }
    
    return true;
}

bool MessageParser::isIcmpHeaderValid(unsigned char* packet, size_t packetSize, uint16_t id, uint16_t seqNumber) {

    // Check the size of the packet
    size_t controlSize = 0;
    if (this->type == AF_INET) controlSize += IPV4_HEADER_SIZE;
    if (packetSize < controlSize + ICMP_HEADER_SIZE) return false;

    // Validate ICMP header independent on IP version
    if (this->type == AF_INET) {
        // Get ICMPv4 header
        struct icmphdr icmpHeader;
        memcpy(&icmpHeader, packet + IPV4_HEADER_SIZE, sizeof(struct icmphdr));

        // Validate fields of ICMP header
        if (icmpHeader.type != ICMP_ECHOREPLY) return false;
        if (icmpHeader.code != 0) return false;
        if (ntohs(icmpHeader.un.echo.id) != id) return false;
        if (ntohs(icmpHeader.un.echo.sequence) != seqNumber) return false;
        
        // Validate checksum of ICMP header
        unsigned short receivedChecksum = icmpHeader.checksum;
        ((struct icmphdr*)(packet + IPV4_HEADER_SIZE))->checksum = 0;
        size_t icmpDataLen = packetSize - IPV4_HEADER_SIZE;
        unsigned short calculatedChecksum = this->calculateChecksum(packet + IPV4_HEADER_SIZE, icmpDataLen, true);
        ((struct icmphdr*)(packet + IPV4_HEADER_SIZE))->checksum = receivedChecksum;
        if (receivedChecksum != calculatedChecksum) return false;

    } else {

        // Get ICMPv6 header
        struct icmp6_hdr icmp6Header;
        memcpy(&icmp6Header, packet, sizeof(struct icmp6_hdr));

        // Validate fields of ICMPv6 header
        if (icmp6Header.icmp6_type != ICMP6_ECHO_REPLY) return false;
        if (icmp6Header.icmp6_code != 0) return false;
        if (ntohs(icmp6Header.icmp6_id) != id) return false;
        if (ntohs(icmp6Header.icmp6_seq) != seqNumber) return false;
        
        // Validate checksum of ICMPv6 header
        unsigned short receivedChecksum = icmp6Header.icmp6_cksum;
        ((struct icmp6_hdr*)packet)->icmp6_cksum = 0;
        unsigned short calculatedChecksum = this->calculateChecksum(packet, packetSize, true);
        ((struct icmp6_hdr*)packet)->icmp6_cksum = receivedChecksum;
        if (receivedChecksum != calculatedChecksum) return false;
    }

    return true;
}

MESSAGE_TYPES MessageParser::parseMessage(unsigned char* packet, size_t packetSize, uint16_t id, uint16_t seqNumber, struct CHIPHER_T &outCipherData){
    
    // Check the validity of IPv4 header
    if (this->type == AF_INET && !this->isIpv4HeaderValid(packet, packetSize)) {
        return MESSAGE_TYPES::BAD_TYPE;
    }

    // Check the validity of ICMP header
    if (!this->isIcmpHeaderValid(packet, packetSize, id, seqNumber)) {
        return MESSAGE_TYPES::BAD_TYPE;
    }

    // Set begin of payload
    size_t parseIndex = ICMP_HEADER_SIZE;
    if (this->type == AF_INET) { 
        parseIndex += IPV4_HEADER_SIZE;
    }

    // Check the minimal size of the payload:
    if (packetSize < parseIndex + PROTOCOL_MESS_TYPE_SIZE + CHECKSUM_PAYLOAD) {
        return MESSAGE_TYPES::BAD_TYPE;
    }

    // Get type of the message of the protocol
    uint16_t msgTypeNet = 0;
    memcpy(&msgTypeNet, &packet[parseIndex], PROTOCOL_MESS_TYPE_SIZE);
    MESSAGE_TYPES msgType = (MESSAGE_TYPES)ntohs(msgTypeNet);
    parseIndex += PROTOCOL_MESS_TYPE_SIZE;

    /**
     * For DATA -> Parse the data from packet -> Payload data <-> chipher data
     * For HELLO -> Parse the file name -> Payload data <-> file name 
     */
    if (msgType == MESSAGE_TYPES::DATA || msgType == MESSAGE_TYPES::HELLO) {

        // Check the minimal size of the packet for DATA | ERR | HELLO message
        size_t minimalSize = parseIndex + DATA_LENGTH_SIZE + INITIALIZE_VECTOR_SIZE + CHECKSUM_PAYLOAD;
        if (packetSize < minimalSize) {
            return MESSAGE_TYPES::BAD_TYPE;
        }

        // Clean the output structure
        memset(&outCipherData, 0, sizeof(CHIPHER_T));
        
        // Parse the length of the encrypted data
        uint32_t totalLenNet = 0;
        memcpy(&totalLenNet, &packet[parseIndex], DATA_LENGTH_SIZE);
        outCipherData.totalLen = ntohl(totalLenNet);
        parseIndex += DATA_LENGTH_SIZE;

        // Store the length of data as size_t
        size_t dataLength = outCipherData.totalLen;

        // Check overflow
        if (dataLength > sizeof(outCipherData.data)) {
            return MESSAGE_TYPES::BAD_TYPE;
        }

        // Calculate the expected size of packet
        size_t expectedSizeOfPacket = minimalSize + dataLength;

        // Check if the expected size is right
        if (packetSize != expectedSizeOfPacket) {
            return MESSAGE_TYPES::BAD_TYPE;
        }
        
        // Parse encrypted data
        memcpy(outCipherData.data, &packet[parseIndex], dataLength);
        parseIndex += dataLength;
        
        // Parse initialization vector
        memcpy(outCipherData.iv, &packet[parseIndex], INITIALIZE_VECTOR_SIZE);
        parseIndex += INITIALIZE_VECTOR_SIZE;
    }
    /**
     * For HELLO_ACK, DATA_ACK, FIN, FIN_ACK, FIN_ERR messages -> No chipher data in payload
     */
    else if (msgType == MESSAGE_TYPES::HELLO_ACK || msgType == MESSAGE_TYPES::DATA_ACK  || msgType == MESSAGE_TYPES::FIN || msgType == MESSAGE_TYPES::FIN_ACK  || msgType == MESSAGE_TYPES::FIN_ERR) {

        // Calculate the expected size of packet
        size_t expectedSizeOfPacket = parseIndex + CHECKSUM_PAYLOAD;
        // Check if the expected size is right
        if (packetSize != expectedSizeOfPacket) {
            return MESSAGE_TYPES::BAD_TYPE;
        }
    }
    else {
        return MESSAGE_TYPES::BAD_TYPE;
    }
    
    // Parse checksum of the payload
    uint32_t receivedChecksumNet = 0;
    memcpy(&receivedChecksumNet, &packet[parseIndex], CHECKSUM_PAYLOAD);
    unsigned short receivedChecksum = (unsigned short)ntohl(receivedChecksumNet);

    // Set payload index for calculate checksum
    size_t payloadIndex = ICMP_HEADER_SIZE;
    if (this->type == AF_INET) {
        payloadIndex += IPV4_HEADER_SIZE;
    }

    // Set payload length
    size_t payloadLength = parseIndex - payloadIndex;
    unsigned short calculatedChecksum = this->calculateChecksum(&packet[payloadIndex], payloadLength, false);
    
    // Check the checksum of the payload
    if (receivedChecksum != calculatedChecksum) {
        return MESSAGE_TYPES::BAD_TYPE;
    }

    return msgType;
}


