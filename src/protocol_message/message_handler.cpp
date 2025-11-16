/**
 * @file message_handler.cpp
 * @author Martin Zůbek
 * @date 2025-10-20
 * @brief Source file with implementation of methods from MessageHandler class. This methods are used in both derived classes MessageCreator and MessageParser.
 *        There are methods for creating Ipv4 header, ICMPs headers and calculating checksums.
 */
#include <iostream>
#include <cstring>
#include <variant>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/icmp6.h>
#include <netinet/ip_icmp.h>
#include "../../inc/protocol_message/message_handler.hpp"
#include "../../inc/types/protocol_definitions.hpp"
#include "../../inc/types/pseudo_header.hpp"


// Constructor of the MessageHandler class
MessageHandler::MessageHandler(int type, 
                               std::variant<struct sockaddr_in, struct sockaddr_in6> destAddress,
                               std::variant<struct sockaddr_in, struct sockaddr_in6> srcAddress) {
    this->type = type;
    this->destAddress = destAddress;
    this->srcAddress = srcAddress;
}

/**
 * @note This code was taken and edit from my previous project.
 * 
 * Reference:
 *   Zůbek, M. First Project for the IPK Course (L4 Scanner, Variant OMEGA).
 *   Brno: FIT BUT, 2025. Unpublished software, private GitHub repository.
 */
unsigned short MessageHandler::calculateChecksum(unsigned char* pdu, size_t dataLen, bool icmpChecksum){
     // Checksum
    unsigned long checksum = 0;

    if(icmpChecksum && this->type == AF_INET6){
        struct checkSumPseudoHdrIpv6 pseudoHeader;
        memset(&pseudoHeader, 0, sizeof(pseudoHeader));

        struct sockaddr_in6 srcAddr = std::get<struct sockaddr_in6>(this->srcAddress);
        struct sockaddr_in6 destAddr = std::get<struct sockaddr_in6>(this->destAddress);

        memcpy(&pseudoHeader.src, &srcAddr.sin6_addr, sizeof(struct in6_addr));
        memcpy(&pseudoHeader.dst, &destAddr.sin6_addr, sizeof(struct in6_addr));
        pseudoHeader.length = htonl(dataLen);
        pseudoHeader.next_header = IPPROTO_ICMPV6;

        // Calculate checksum for pseudo header
        unsigned char* ptrIndex = (unsigned char*)&pseudoHeader;
        size_t offset = 0;
        while (offset < sizeof(pseudoHeader)){
            checksum += *(unsigned short*)(ptrIndex + offset);
            offset += 2;
        }
    }
    
    // Offset
    size_t offset = 0;
    // Calculate checksum
    while (offset < dataLen-1){
        checksum += *(unsigned short *)&pdu[offset];
        // 16 bits -> 2 bytes
        offset += 2;
    }
    // If data length is odd, add last byte
    if (dataLen%2) checksum += (unsigned char) pdu[offset];
    // Add carry
    while (checksum >> 16) checksum = (checksum & 0xFFFF) + (checksum >> 16);
    // Return checksum
    return (unsigned short) ~checksum;
}

void MessageHandler::setDestAddress(std::variant<struct sockaddr_in, struct sockaddr_in6> destAddress) {
    this->destAddress = destAddress;
}

void MessageHandler::setSrcAddress(std::variant<struct sockaddr_in, struct sockaddr_in6> srcAddress) {
    this->srcAddress = srcAddress;
}

void MessageHandler::setType(int type) {
    this->type = type;
}

void MessageHandler::createIcmpHeader(unsigned char* packet, size_t &copyIndex, uint16_t id, uint16_t seqNumber, uint8_t icmpType, uint8_t icmp6Type, uint16_t totalPacketSize) {
    // Create ICMP header for IPv4
    if(this->type == AF_INET) {
        struct icmphdr icmpHdr;
        memset(&icmpHdr, 0, sizeof(struct icmphdr));
        icmpHdr.type = icmpType;
        icmpHdr.code = 0;
        icmpHdr.un.echo.id = htons(id);
        icmpHdr.un.echo.sequence = htons(seqNumber);

        // Create IPv4 header
        struct iphdr ipHeader = this->createIpv4Header(id, totalPacketSize);
        memcpy(&packet[copyIndex], &ipHeader, IPV4_HEADER_SIZE);
        copyIndex += IPV4_HEADER_SIZE;
        
        memcpy(&packet[copyIndex], &icmpHdr, sizeof(struct icmphdr));
    // Create ICMP header for IPv6
    } else {
        struct icmp6_hdr icmpHdr;
        memset(&icmpHdr, 0, sizeof(icmpHdr));
        icmpHdr.icmp6_type = icmp6Type;
        icmpHdr.icmp6_code = 0;
        icmpHdr.icmp6_id = htons(id);
        icmpHdr.icmp6_seq = htons(seqNumber);
        
        memcpy(&packet[copyIndex], &icmpHdr, sizeof(struct icmp6_hdr));
    }

    // Update copy index for size of ICMP header
    copyIndex += ICMP_HEADER_SIZE;
}

void MessageHandler::calculatePayloadChecksum(unsigned char* packet, size_t &copyIndex) {
    unsigned short payloadChecksum = 0;
    // ICMP header is not part of the payload
    size_t deleteHeader = ICMP_HEADER_SIZE;
    // If IPv4, IPv4 header is not part of the payload
    if(this->type == AF_INET) deleteHeader += IPV4_HEADER_SIZE;

    // Calculate checksum of the payload and store it
    payloadChecksum = this->calculateChecksum(&packet[deleteHeader], copyIndex - deleteHeader, false);
    uint32_t payloadChecksumNet = htonl((uint32_t)payloadChecksum);
    memcpy(&packet[copyIndex], &payloadChecksumNet, CHECKSUM_PAYLOAD);
    copyIndex += CHECKSUM_PAYLOAD;
}

void MessageHandler::calculateIcmpChecksum(unsigned char* packet, size_t copyIndex) {
    // Calculate ICMP checksum independently for IPv4 and IPv6
    if(this->type == AF_INET){
        // Calculate checksum without IPv4 header
        unsigned short icmpChecksum = this->calculateChecksum(&packet[IPV4_HEADER_SIZE], copyIndex - IPV4_HEADER_SIZE, true);
        ((struct icmphdr*)&packet[IPV4_HEADER_SIZE])->checksum = icmpChecksum;
    }else if(this->type == AF_INET6){
        // Calculate checksum for entire packet -> Ipv6 header will be added by kernel
        unsigned short icmpChecksum = this->calculateChecksum(packet, copyIndex, true);
        ((struct icmp6_hdr*)&packet[0])->icmp6_cksum = icmpChecksum;
    }
}

struct iphdr MessageHandler::createIpv4Header(uint16_t id, uint16_t len){
    // Create IPv4 header
    struct iphdr ipHeader;
    memset(&ipHeader, 0, sizeof(struct iphdr));
    ipHeader.version = 4; 
    ipHeader.ihl = 5;
    ipHeader.tos = 0;     
    ipHeader.tot_len = htons(len); 
    ipHeader.id = htons(id); 
    ipHeader.frag_off = 0; 
    ipHeader.ttl = 64;
    ipHeader.protocol = IPPROTO_ICMP; 
    ipHeader.check = 0;

    // Set source and destination addresses
    struct sockaddr_in srcAddr = std::get<struct sockaddr_in>(this->srcAddress);
    struct sockaddr_in destAddr = std::get<struct sockaddr_in>(this->destAddress);
    ipHeader.saddr = srcAddr.sin_addr.s_addr; 
    ipHeader.daddr = destAddr.sin_addr.s_addr;

    // Calculate checksum
    ipHeader.check = this->calculateChecksum((unsigned char*)&ipHeader, sizeof(ipHeader),false);

    return ipHeader;
}