#ifndef MESSAGE_HANDLER_HPP
#define MESSAGE_HANDLER_HPP

#include <iostream>
#include <variant>
#include <netinet/in.h>
#include <cstdint>

class MessageHandler {
    protected:
        int type;
        std::variant<struct sockaddr_in, struct sockaddr_in6> destAddress;
        std::variant<struct sockaddr_in, struct sockaddr_in6> srcAddress;
        
        unsigned short calculateChecksum(unsigned char* pdu, size_t dataLen, bool icmpChecksum);
        void createIcmpHeader(unsigned char* packet, size_t &copyIndex, uint16_t id, uint16_t seqNumber, uint8_t icmpType, uint8_t icmp6Type, uint16_t totalPacketSize);
        void calculatePayloadChecksum(unsigned char* packet, size_t &copyIndex);
        void calculateIcmpChecksum(unsigned char* packet, size_t dataLen);
        struct iphdr createIpv4Header(uint16_t id, uint16_t len);

        
    public:
        MessageHandler(int type, 
                      std::variant<struct sockaddr_in, struct sockaddr_in6> destAddress,
                      std::variant<struct sockaddr_in, struct sockaddr_in6> srcAddress);
        
        virtual ~MessageHandler() = default;
        
        void setDestAddress(std::variant<struct sockaddr_in, struct sockaddr_in6> destAddress);
        void setSrcAddress(std::variant<struct sockaddr_in, struct sockaddr_in6> srcAddress);
        void setType(int type);
};

#endif
