/**
 * @file pseudo_header.hpp
 * @author Martin Zůbek
 * @date 2025-10-20
 * @brief Header file with definition of struct for pseudo header for checksum calculation for IPv6. 
 * @note This code was taken and edit from my previous project for subject IPK (first project) at VUT FIT in 2025.
 */

#ifndef  PSEUDO_HEADER_HPP
#define PSEUDO_HEADER_HPP

#include <cstdint>
#include <netinet/in.h>

/**
 * @brief Struct for creating pseudo header for checksum calculation for IPv6
 */
struct checkSumPseudoHdrIpv6 {
    // Source address
    struct in6_addr src;
    // Destination address
    struct in6_addr dst;
    // Length of the protocol header
    uint32_t length;
    // Zero field
    uint8_t zero[3];
    // Next header -> protocol number
    uint8_t next_header;
};

#endif // PSEUDO_HEADER_HPP
