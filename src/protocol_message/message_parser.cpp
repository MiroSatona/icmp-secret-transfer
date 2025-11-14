#include <iostream>
#include <stdexcept>
#include <cstring>
#include <variant>
#include <netinet/ip_icmp.h>
#include <netinet/icmp6.h>
#include <arpa/inet.h>
#include "../../inc/protocol_message/message_handler.hpp"
#include "../../inc/protocol_message/message_parser.hpp"
#include "../../inc/types/protocol_definitions.hpp"
#include "../../inc/types/struct_cipher.hpp"

MessageParser::MessageParser()
    : MessageHandler(
        AF_UNSPEC,
        std::variant<struct sockaddr_in, struct sockaddr_in6>{sockaddr_in{}},
        std::variant<struct sockaddr_in, struct sockaddr_in6>{sockaddr_in{}}
){}

bool MessageParser::isIpv4HeaderValid(unsigned char* packet, size_t packetSize) {

    if(packetSize < IPV4_HEADER_SIZE) return false;
    
    struct iphdr ipHeader;
    memset(&ipHeader, 0, sizeof(struct iphdr));
    memcpy(&ipHeader, packet, sizeof(struct iphdr));
    
    if(ipHeader.version != 4) return false;
    if(ipHeader.ihl < 5) return false;
    if(ipHeader.protocol != IPPROTO_ICMP) return false;
      
    struct iphdr ipHeaderTmp;
    memcpy(&ipHeaderTmp, &ipHeader, sizeof(struct iphdr));
    ipHeaderTmp.check = 0;
    unsigned short calculatedChecksum = this->calculateChecksum((unsigned char*)&ipHeaderTmp, IPV4_HEADER_SIZE, false);
    
    if(ipHeader.check != calculatedChecksum) return false;
    
    struct sockaddr_in destAddr = std::get<struct sockaddr_in>(this->destAddress);
    struct sockaddr_in srcAddr = std::get<struct sockaddr_in>(this->srcAddress);
    
    if(memcmp(&ipHeader.daddr, &destAddr.sin_addr.s_addr, sizeof(uint32_t)) || memcmp(&ipHeader.saddr, &srcAddr.sin_addr.s_addr, sizeof(uint32_t))) {
        return false;
    }
    
    return true;
}

bool MessageParser::isIcmpHeaderValid(unsigned char* packet, size_t packetSize, uint16_t id, uint16_t seqNumber) {
    size_t controlSize = 0;
    if (this->type == AF_INET) controlSize += IPV4_HEADER_SIZE;
    if (packetSize < controlSize + ICMP_HEADER_SIZE) return false;

    if (this->type == AF_INET) {
        struct icmphdr icmpHeader;
        memcpy(&icmpHeader, packet + IPV4_HEADER_SIZE, sizeof(struct icmphdr));

        if (ntohs(icmpHeader.un.echo.id) != id) return false;
        if (ntohs(icmpHeader.un.echo.sequence) != seqNumber) return false;

        unsigned short receivedChecksum = icmpHeader.checksum;

        ((struct icmphdr*)(packet + IPV4_HEADER_SIZE))->checksum = 0;
        size_t icmpDataLen = packetSize - IPV4_HEADER_SIZE;

        unsigned short calculatedChecksum = this->calculateChecksum(packet + IPV4_HEADER_SIZE, icmpDataLen, true);

        ((struct icmphdr*)(packet + IPV4_HEADER_SIZE))->checksum = receivedChecksum;

        if (receivedChecksum != calculatedChecksum) return false;

    } else {
        struct icmp6_hdr icmp6Header;
        memcpy(&icmp6Header, packet, sizeof(struct icmp6_hdr));

        if (ntohs(icmp6Header.icmp6_id) != id) return false;
        if (ntohs(icmp6Header.icmp6_seq) != seqNumber) return false;

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
    MESSAGE_TYPES msgType = MESSAGE_TYPES::BAD_TYPE;
    memcpy(&msgType, &packet[parseIndex], PROTOCOL_MESS_TYPE_SIZE);
    parseIndex += PROTOCOL_MESS_TYPE_SIZE;

    /**
     * For DATA -> Parse the data from packet
     * For ERR  -> Parse the error from client/server
     * For HELLO -> Parse the file name (šifrovaně)
     */
    if (msgType == MESSAGE_TYPES::DATA || msgType == MESSAGE_TYPES::ERR || msgType == MESSAGE_TYPES::HELLO) {

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
     * For HELLO_ACK, DATA_ACK, FIN, FIN_ACK, WAIT messages
     */
    else if (msgType == MESSAGE_TYPES::HELLO_ACK || msgType == MESSAGE_TYPES::DATA_ACK  || msgType == MESSAGE_TYPES::FIN || msgType == MESSAGE_TYPES::FIN_ACK   ||  msgType == MESSAGE_TYPES::WAIT) {

        size_t expectedSizeOfPacket = parseIndex + CHECKSUM_PAYLOAD;

        if (packetSize != expectedSizeOfPacket) {
            return MESSAGE_TYPES::BAD_TYPE;
        }
    }
    else {
        return MESSAGE_TYPES::BAD_TYPE;
    }
    
    // Parse checksum of the payload
    unsigned short receivedChecksumNet = 0;
    memcpy(&receivedChecksumNet, &packet[parseIndex], CHECKSUM_PAYLOAD);
    unsigned short receivedChecksum = ntohs(receivedChecksumNet);

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


