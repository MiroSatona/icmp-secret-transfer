 /**
 * @file message_creator.cpp
 * @author Martin Zůbek
 * @date 2025-10-20
 * @brief Implementation of MessageCreator class for creating protocol messages. There are methods for creating messages with and without chipher data.
 */
#include <iostream>
#include <stdexcept>
#include <cstring>
#include <cstddef> 
#include <string>
#include <variant>
#include <netinet/icmp6.h>
#include <netinet/ip_icmp.h>
#include "../../inc/protocol_message/message_creator.hpp"
#include "../../inc/types/protocol_definitions.hpp"
#include "../../inc/types/pseudo_header.hpp"

// Constructor of MessageCreator class
MessageCreator::MessageCreator(int type, std::variant<struct sockaddr_in, struct sockaddr_in6> destAddress,std::variant<struct sockaddr_in, struct sockaddr_in6> srcAddress) 
    : MessageHandler(type, destAddress, srcAddress) {
}


unsigned char* MessageCreator::createMessageWithData(CHIPHER_T dataToSend, uint16_t id, uint16_t seqNumber, MESSAGE_TYPES messType, size_t& outSize, uint8_t icmpType, uint8_t icmp6Type) {
    /**
     * Calculate payload size:
     * Message type -> 2 bytes
     * Data length -> 4 bytes
     * Data -> variable length
     * Initialization vector -> 16 bytes
     * Checksum of payload -> 2 bytes
     * 
     */
    size_t payloadSize = PROTOCOL_MESS_TYPE_SIZE + 
                         dataToSend.totalLen +
                         DATA_LENGTH_SIZE +
                         INITIALIZE_VECTOR_SIZE +
                         CHECKSUM_PAYLOAD;

    // Calculate total packet size independent on IP version
    size_t totalPacketSize = payloadSize + ICMP_HEADER_SIZE;
    if(this->type == AF_INET) {
        totalPacketSize += IPV4_HEADER_SIZE;
    }

    // Create the new packet with rifht size
    unsigned char *packet = new unsigned char[totalPacketSize];
    memset(packet, 0, totalPacketSize);
    
    // Init index of the packat
    size_t copyIndex = 0;
    
    // Create ICMP header
    this->createIcmpHeader(packet, copyIndex, id, seqNumber, icmpType, icmp6Type, totalPacketSize);

    // Set message type
    uint16_t msgType = htons(messType);
    memcpy(&packet[copyIndex], &msgType, PROTOCOL_MESS_TYPE_SIZE);
    copyIndex += PROTOCOL_MESS_TYPE_SIZE;

    // Set data length
    uint32_t totalLen = htonl(dataToSend.totalLen);
    memcpy(&packet[copyIndex], &totalLen, DATA_LENGTH_SIZE);
    copyIndex += DATA_LENGTH_SIZE;

    // Copy data
    memcpy(&packet[copyIndex], dataToSend.data, dataToSend.totalLen);
    copyIndex += dataToSend.totalLen;

    // Copy initialization vector
    memcpy(&packet[copyIndex], dataToSend.iv, INITIALIZE_VECTOR_SIZE);
    copyIndex += INITIALIZE_VECTOR_SIZE;

    // Calculate checksum of payload
    this->calculatePayloadChecksum(packet, copyIndex);
    // Calculate ICMP checksum
    this->calculateIcmpChecksum(packet, copyIndex);

    // Set output size and return the created packet
    outSize = copyIndex;
    return packet;
}


unsigned char* MessageCreator::createMessageWithNoData(uint16_t id, uint16_t seqNumber, MESSAGE_TYPES msgType, size_t& outSize, uint8_t icmpType, uint8_t icmp6Type) {
    /** Calculate total packet size independent on IP version 
     * 
     * Message type -> 2 bytes
     * Checksum of payload -> 2 bytes
     * ICMP header -> 8 bytes
     * IPv4 header -> 20 bytes
    */
    size_t totalPacketSize =  PROTOCOL_MESS_TYPE_SIZE + CHECKSUM_PAYLOAD + ICMP_HEADER_SIZE;
    if(this->type == AF_INET) {
        totalPacketSize += IPV4_HEADER_SIZE;
    }

    // Create the new packet with right size
    unsigned char *packet = new unsigned char[totalPacketSize];
    memset(packet, 0, totalPacketSize);

    // Init index of the packet
    size_t copyIndex = 0;

    // Create ICMP header
    this->createIcmpHeader(packet, copyIndex, id, seqNumber, icmpType, icmp6Type, totalPacketSize);

    // Set message type
    uint16_t messageTypeNet = htons(msgType);
    memcpy(&packet[copyIndex], &messageTypeNet, PROTOCOL_MESS_TYPE_SIZE);
    copyIndex += PROTOCOL_MESS_TYPE_SIZE;

    // Calculate checksum of payload
    this->calculatePayloadChecksum(packet, copyIndex);

    // Calculate ICMP checksum
    this->calculateIcmpChecksum(packet, copyIndex);

    // Set output size and return the created packet
    outSize = copyIndex;
    return packet;
}




