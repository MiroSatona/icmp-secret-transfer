/**
 * @file message_handler.hpp
 * @author Martin Zůbek
 * @date 2025-10-20
 * @brief Header file with definition of MessageHandler class. This class is base class for MessageCreator and MessageParser classes in [src|inc]/protocol_message.
 *        This class contains the methods wich is used in both derived classes.
 */
#ifndef MESSAGE_HANDLER_HPP
#define MESSAGE_HANDLER_HPP

#include <iostream>
#include <variant>
#include <netinet/in.h>
#include <cstdint>

/**
 * @class MessageHandler
 * @brief Base class for MessageCreator and MessageParser classes.
 */
class MessageHandler {
    protected:
        /// @brief Type of Ip version (Ipv4 or Ipv6) represented by AF_INET or AF_INET6 from <arpa/inet.h>.
        int type;
        /// @brief Destination address (IPv4 or IPv6) represented by sockaddr_in or sockaddr_in6 from <netinet/in.h>.
        std::variant<struct sockaddr_in, struct sockaddr_in6> destAddress;
        /// @brief Source address (IPv4 or IPv6) represented by sockaddr_in or sockaddr_in6 from <netinet/in.h>.
        std::variant<struct sockaddr_in, struct sockaddr_in6> srcAddress;
        
        /**
         * @brief Method for calculate checksum.
         * 
         * @param pdu Data unit fot wich will be checksum calculated.
         * @param dataLen Length of the data unit.
         * @param icmpChecksum Flag indicating if ICMP checksum should be calculated.
         * @return unsigned short The calculated checksum.
         */
        unsigned short calculateChecksum(unsigned char* pdu, size_t dataLen, bool icmpChecksum);
        /**
         * @brief Method for creating ICMP header.
         * 
         * @param packet Pointer to the packet where the ICMP header will be created.
         * @param copyIndex Index if packet where will be ICMP header stored.
         * @param id Identifier for ICMP header.
         * @param seqNumber Sequence number for ICMP header.
         * @param icmpType ICMP type for IPv4.
         * @param icmp6Type ICMP type for IPv6.
         * @param totalPacketSize Total size of the packet.
         */
        void createIcmpHeader(unsigned char* packet, size_t &copyIndex, uint16_t id, uint16_t seqNumber, uint8_t icmpType, uint8_t icmp6Type, uint16_t totalPacketSize);
        /**
         * @brief Method for calculating checksum of the payload. 
         * 
         * @param packet Pointer to the packet where is the payload stored.
         * @param copyIndex Index of the packet where the checksum will be stored.
         */
        void calculatePayloadChecksum(unsigned char* packet, size_t &copyIndex);
        /**
         * @brief Method for calculating ICMP checksum.
         * 
         * @param packet Pointer to the packet where is the stored ICMP message.
         * @param dataLen Length of the ICMP message.
         */
        void calculateIcmpChecksum(unsigned char* packet, size_t dataLen);
        /**
         * @brief Method for creating IPv4 header.
         * 
         * @param id Identifier for IPv4 header.
         * @param len Length of the IPv4 header.
         * @return struct iphdr Created IPv4 header.
         */
        struct iphdr createIpv4Header(uint16_t id, uint16_t len);

    public:
        /**
         * @brief Constructor of the MessageHandler class.
         * 
         * @param type Type of Ip version (Ipv4 or Ipv6) represented by AF_INET or AF_INET6 from <arpa/inet.h>.
         * @param destAddress Destination address (IPv4 or IPv6) represented by sockaddr_in or sockaddr_in6 from <netinet/in.h>.
         * @param srcAddress Source address (IPv4 or IPv6) represented by sockaddr_in or sockaddr_in6 from <netinet/in.h>.
         */
        MessageHandler(int type, 
                      std::variant<struct sockaddr_in, struct sockaddr_in6> destAddress,
                      std::variant<struct sockaddr_in, struct sockaddr_in6> srcAddress);
        /**
         * @brief Virtual destructor of the MessageHandler class.
         */
        virtual ~MessageHandler() = default;
        /**
         * @brief Setter for destination address.
         * 
         * @param destAddress Destination address (IPv4 or IPv6) represented by sockaddr_in or sockaddr_in6 from <netinet/in.h>.
         */
        void setDestAddress(std::variant<struct sockaddr_in, struct sockaddr_in6> destAddress);
        /**
         * @brief Setter for source address.
         * 
         * @param srcAddress Source address (IPv4 or IPv6) represented by sockaddr_in or sockaddr_in6 from <netinet/in.h>.
         */
        void setSrcAddress(std::variant<struct sockaddr_in, struct sockaddr_in6> srcAddress);
        /**
         * @brief Setter for type of IP version.
         * 
         * @param type Type of Ip version (Ipv4 or Ipv6) represented by AF_INET or AF_INET6 from <arpa/inet.h>.
         */
        void setType(int type);
};

#endif
