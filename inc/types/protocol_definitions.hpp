/**
 * @file protocol_definitions.hpp
 * @author Martin Zůbel
 * @date 2025-10-17
 * @brief Header file with protocol message types and states of client and server.
 */


#ifndef PROTOCOL_DEFINITIONS_HPP
#define PROTOCOL_DEFINITIONS_HPP

#include <iostream>

// Application protocol port
#define APP_PROTOCOL_PORT 50001
// Maximum size of a packet
#define MAX_PACKET_SIZE 1500
// Lengtht of ipv4 header
#define IPV4_HEADER_SIZE 20
// Lengtht of protocol message type
#define PROTOCOL_MESS_TYPE_SIZE 2
// Length of initialize vector
#define INITIALIZE_VECTOR_SIZE 16
// Length of padding flag
#define PADDING_FLAG_SIZE 1
// Length of padding length
#define PADDING_LENGTH_SIZE 4
// Length of data length
#define DATA_LENGTH_SIZE 4

/**
 * @enum MESSAGE_TYPES
 * @brief Enumeration of message types used in the protocol.
 */
enum MESSAGE_TYPES{
    HELLO = 0x01,
    HELLO_ACK = 0x02,
    DATA = 0x03,
    DATA_ACK = 0x04,
    DATA_ERR = 0x05,
    FIN = 0x06,
    FIN_ACK = 0x07,
    ERR = 0x08
};


/**
 * @enum CLIENT_STATE
 * @brief Enumeration of client states in the protocol. 
 */
enum CLIENT_STATE{
    AUTH_STATE,
    WAIT_STATE,
    SENDING_STATE,
    ERROR_STATE,
    SERVER_ERROR_STATE,
    END_STATE
};

/**
 * @enum PADDING_FLAGS
 * @brief Enumeration of server states in the protocol. 
 */
enum SERVER_STATE{
    LISTENING_STATE,
    RECEIVING_STATE,
    ERROR_STATE
};

#endif // PROTOCOL_DEFINITIONS_HPP