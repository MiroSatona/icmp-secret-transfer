/**
 * @file union_address.hpp
 * @author Martin Zůbek
 * @date 2025-09-26
 * @brief Header file with definition of union ipvSocketAddress. This union is used for resolved hostname/ipv4/ipv6 address in the program.
 *        Is used for easy storage of both types of the ip protocols.
 */
#ifndef UNION_ADDRESS_HPP
#define UNION_ADDRESS_HPP

#include <sys/socket.h>
#include <netinet/in.h>

/**
 * @union ipvSocketAddress
 * @brief Union for storing both IPv4 and IPv6 socket address structures.
 */
union ipvSocketAddress{
    struct sockaddr_in ipv4;
    struct sockaddr_in6 ipv6;
};

#endif // UNION_ADDRESS_HPP