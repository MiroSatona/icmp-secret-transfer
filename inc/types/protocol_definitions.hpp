/**
 * @file protocol_definitions.hpp
 * @author Martin Zůbel
 * @date 2025-10-17
 * @brief Header file with protocol message types and states of client and server.
 */


#ifndef PROTOCOL_DEFINITIONS_HPP
#define PROTOCOL_DEFINITIONS_HPP

#include <iostream>
#include <cstdint>

// Application protocol port
#define APP_PROTOCOL_PORT 50001
// Maximum size of a packet in bytes
#define MAX_PACKET_SIZE 1500
// Length of ipv4 header in bytes
#define IPV4_HEADER_SIZE 20
// Length of icmpv[4|6] header in bytes
#define ICMP_HEADER_SIZE 8
// Length of protocol message type in bytes
#define PROTOCOL_MESS_TYPE_SIZE 2
// Length of initialize vector in bytes
#define INITIALIZE_VECTOR_SIZE 16
// Length of payload checksum in bytes
#define CHECKSUM_PAYLOAD 4
// Length of data length in bytes
#define DATA_LENGTH_SIZE 4
// Length of payload of acknowledgment messages in bytes
#define ACKNOWLEDGMENT_MESSAGES_PAYLOAD_LENGTH 4

/**
 * @enum MESSAGE_TYPES
 * @brief Enumeration of message types used in the protocol.
 */
enum MESSAGE_TYPES : uint16_t{
    BAD_TYPE = 0x00,
    HELLO = 0x01,
    HELLO_ACK = 0x02,
    DATA = 0x03,
    DATA_ACK = 0x04,
    FIN = 0x06,
    FIN_ACK = 0x07,
    ERR = 0x08,
    WAIT = 0X09
};


/**
 * @enum CLIENT_STATE
 * @brief Enumeration of client states in the protocol. 
 */
enum CLIENT_STATE{
    AUTH_STATE,
    WAIT_STATE,
    SENDING_STATE,
    CLIENT_ERROR_STATE,
    SERVER_ERROR_STATE,
    FIN_STATE
};

/**
 * @enum SERVER_STATE
 * @brief Enumeration of server states in the protocol. 
 */
enum SERVER_STATE{
    LISTENING_STATE,
    RECEIVING_STATE,
    ERROR_STATE,
    ENDING_STATE
};

#endif // PROTOCOL_DEFINITIONS_HPP