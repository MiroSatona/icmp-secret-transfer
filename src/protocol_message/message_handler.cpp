#include <iostream>
#include <cstring>
#include <variant>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/ip_icmp.h>
#include <netinet/icmp6.h>
#include "../../inc/protocol_message/message_handler.hpp"
#include "../../inc/types/protocol_definitions.hpp"
#include "../../inc/types/pseudo_header.hpp"


MessageHandler::MessageHandler(int type, 
                               std::variant<struct sockaddr_in, struct sockaddr_in6> destAddress,
                               std::variant<struct sockaddr_in, struct sockaddr_in6> srcAddress) {
    this->type = type;
    this->destAddress = destAddress;
    this->srcAddress = srcAddress;
}

/**
 * @note This code was taken and edit from my previous project for subject IPK (first project) at VUT FIT in 2025.
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
    if(this->type == AF_INET) {
        struct icmphdr icmpHdr;
        memset(&icmpHdr, 0, sizeof(struct icmphdr));
        icmpHdr.type = icmpType;
        icmpHdr.code = 0;
        icmpHdr.un.echo.id = htons(id);
        icmpHdr.un.echo.sequence = htons(seqNumber);

        struct iphdr ipHeader = this->createIpv4Header(id, totalPacketSize);
        memcpy(&packet[copyIndex], &ipHeader, IPV4_HEADER_SIZE);
        copyIndex += IPV4_HEADER_SIZE;
        
        memcpy(&packet[copyIndex], &icmpHdr, sizeof(struct icmphdr));
    } else {
        struct icmp6_hdr icmpHdr;
        memset(&icmpHdr, 0, sizeof(icmpHdr));
        icmpHdr.icmp6_type = icmp6Type;
        icmpHdr.icmp6_code = 0;
        icmpHdr.icmp6_id = htons(id);
        icmpHdr.icmp6_seq = htons(seqNumber);
        
        memcpy(&packet[copyIndex], &icmpHdr, sizeof(struct icmp6_hdr));
    }

    copyIndex += ICMP_HEADER_SIZE;
}

void MessageHandler::calculatePayloadChecksum(unsigned char* packet, size_t &copyIndex) {
    unsigned short payloadChecksum = 0;
    size_t deleteHeader = ICMP_HEADER_SIZE;
    if(this->type == AF_INET) deleteHeader += IPV4_HEADER_SIZE;

    payloadChecksum = this->calculateChecksum(&packet[deleteHeader], copyIndex - deleteHeader, false);
    uint32_t payloadChecksumNet = htonl(payloadChecksum);
    memcpy(&packet[copyIndex], &payloadChecksumNet, CHECKSUM_PAYLOAD);
    copyIndex += CHECKSUM_PAYLOAD;
}

void MessageHandler::calculateIcmpChecksum(unsigned char* packet, size_t copyIndex) {
    if(this->type == AF_INET){
        unsigned short icmpChecksum = this->calculateChecksum(&packet[IPV4_HEADER_SIZE], copyIndex - IPV4_HEADER_SIZE, true);
        ((struct icmphdr*)&packet[IPV4_HEADER_SIZE])->checksum = icmpChecksum;
    }else if(this->type == AF_INET6){
        unsigned short icmpChecksum = this->calculateChecksum(packet, copyIndex - IPV4_HEADER_SIZE, true);
        ((struct icmp6_hdr*)&packet[0])->icmp6_cksum = icmpChecksum;
    }
}

struct iphdr MessageHandler::createIpv4Header(uint16_t id, uint16_t len){
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

    struct sockaddr_in srcAddr = std::get<struct sockaddr_in>(this->srcAddress);
    struct sockaddr_in destAddr = std::get<struct sockaddr_in>(this->destAddress);

    ipHeader.saddr = srcAddr.sin_addr.s_addr; 
    ipHeader.daddr = destAddr.sin_addr.s_addr;

    ipHeader.check = this->calculateChecksum((unsigned char*)&ipHeader, sizeof(ipHeader),false);

    return ipHeader;
}