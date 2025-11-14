#ifndef MESSAGE_CREATOR_HPP
#define MESSAGE_CREATOR_HPP

#include <iostream>
#include <variant>
#include <netinet/in.h>
#include <netinet/ip.h> 
#include "message_handler.hpp"
#include "../types/struct_cipher.hpp"
#include "../types/protocol_definitions.hpp"


class MessageCreator : public MessageHandler {
    public:
        MessageCreator(int type, 
                       std::variant<struct sockaddr_in, struct sockaddr_in6> destAddress,
                       std::variant<struct sockaddr_in, struct sockaddr_in6> srcAddress);
        unsigned char * createMessageWithData(CHIPHER_T dataToSend, uint16_t id, uint16_t seqNumber, MESSAGE_TYPES msgType, size_t& outSize);
        unsigned char * createAckMessage(uint16_t id, uint16_t seqNumber, MESSAGE_TYPES msgType, size_t& outSize);
};

#endif // MESSAGE_CREATOR_HPP