#ifndef MESSAGE_PARSER_HPP
#define MESSAGE_PARSER_HPP

#include <iostream>
#include <variant>
#include <netinet/in.h>
#include <netinet/ip.h> 
#include "../types/struct_cipher.hpp"
#include "../types/protocol_definitions.hpp"
#include "../../inc/protocol_message/message_handler.hpp"



class MessageParser : public MessageHandler{
    private:
        bool isIpv4HeaderValid(unsigned char* packet, size_t packetSize);
        bool isIcmpHeaderValid(unsigned char* packet, size_t packetSize,  uint16_t id, uint16_t seqNumber);
    public:
        MessageParser();
        MESSAGE_TYPES parseMessage(unsigned char* packet, size_t packetSize, uint16_t id, uint16_t seqNumber, struct CHIPHER_T &outCipherData);
};

#endif // MESSAGE_PARSER_HPP