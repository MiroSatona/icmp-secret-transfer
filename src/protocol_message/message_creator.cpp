#include <iostream>
#include <stdexcept>
#include <cstring>
#include <cstddef> 
#include <string>
#include <variant>
#include <netinet/ip_icmp.h>
#include <netinet/icmp6.h>
#include "../../inc/protocol_message/message_creator.hpp"
#include "../../inc/types/protocol_definitions.hpp"
#include "../../inc/types/pseudo_header.hpp"


MessageCreator::MessageCreator(int type, std::variant<struct sockaddr_in, struct sockaddr_in6> destAddress,std::variant<struct sockaddr_in, struct sockaddr_in6> srcAddress) 
    : MessageHandler(type, destAddress, srcAddress) {
}


unsigned char* MessageCreator::createMessageWithData(CHIPHER_T dataToSend, uint16_t id, uint16_t seqNumber, MESSAGE_TYPES messType, size_t& outSize) {

    size_t payloadSize = PROTOCOL_MESS_TYPE_SIZE + 
                         dataToSend.totalLen +
                         DATA_LENGTH_SIZE +
                         INITIALIZE_VECTOR_SIZE +
                         CHECKSUM_PAYLOAD;

    size_t totalPacketSize = payloadSize + ICMP_HEADER_SIZE;
    if(type == AF_INET) {
        totalPacketSize += IPV4_HEADER_SIZE;
    }

    unsigned char *packet = new unsigned char[totalPacketSize];
    memset(packet, 0, totalPacketSize);

    size_t copyIndex = 0;
    
    this->createIcmpHeader(packet, copyIndex, id, seqNumber, ICMP_ECHO, ICMP6_ECHO_REQUEST, totalPacketSize);

    uint16_t msgType = htons(messType);
    memcpy(&packet[copyIndex], &msgType, PROTOCOL_MESS_TYPE_SIZE);
    copyIndex += PROTOCOL_MESS_TYPE_SIZE;

    uint32_t totalLen = htonl(dataToSend.totalLen);
    memcpy(&packet[copyIndex], &totalLen, DATA_LENGTH_SIZE);
    copyIndex += DATA_LENGTH_SIZE;

    memcpy(&packet[copyIndex], dataToSend.data, dataToSend.totalLen);
    copyIndex += dataToSend.totalLen;

    memcpy(&packet[copyIndex], dataToSend.iv, INITIALIZE_VECTOR_SIZE);
    copyIndex += INITIALIZE_VECTOR_SIZE;

   
    this->calculatePayloadChecksum(packet, copyIndex);
    this->calculateIcmpChecksum(packet, copyIndex);

    outSize = copyIndex;
    return packet;
}


unsigned char* MessageCreator::createAckMessage(uint16_t id, uint16_t seqNumber, MESSAGE_TYPES msgType, size_t& outSize) {
    size_t totalPacketSize = ACKNOWLEDGMENT_MESSAGES_PAYLOAD_LENGTH + ICMP_HEADER_SIZE;
    if(type == AF_INET) {
        totalPacketSize += IPV4_HEADER_SIZE;
    }

    unsigned char *packet = new unsigned char[totalPacketSize];
    memset(packet, 0, totalPacketSize);

    size_t copyIndex = 0;
    this->createIcmpHeader(packet, copyIndex, id, seqNumber, ICMP_ECHOREPLY, ICMP6_ECHO_REPLY, totalPacketSize);

    uint16_t messageTypeNet = htons(msgType);
    memcpy(&packet[copyIndex], &messageTypeNet, PROTOCOL_MESS_TYPE_SIZE);
    copyIndex += PROTOCOL_MESS_TYPE_SIZE;

    this->calculatePayloadChecksum(packet, copyIndex);

    this->calculateIcmpChecksum(packet, copyIndex);

    outSize = copyIndex;
    return packet;
}




