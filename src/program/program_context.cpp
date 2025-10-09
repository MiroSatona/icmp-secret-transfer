/**
 * @file program_context.cpp
 * @author Martin Zůbek
 * @date 2025-09-26
 * @brief Header file with definition of ProgramContext class. There are implementations of the methods wich is responsible
 *        for parsing cmd line arguments, resolving hostname/ipv4/ipv6 and setting context of the all program, wich mode will start.
 */
#include <iostream>
#include <stdexcept>
#include <arpa/inet.h>
#include <netdb.h>
#include <cstring>
#include "../../inc/program/abstract_program.hpp"
#include "../../inc/program/program_context.hpp"

ProgramContext::ProgramContext(int argCount, char **argsArray) : 
    program(nullptr), programArguments(argsArray), argumentCount(argCount), context(NOT_SET) {
}

void ProgramContext::getContext(){

    if(this->argumentCount < MIN_ARGUMENTS_COUNT){
        throw std::invalid_argument("Error: Any argument was not provided!");
    }

    // Parsed file pathfrom cmd input
    std::string file = "";
    // Parsed hostname from cmd input
    std::string hostname = "";

    for(int index = 1; index < this->argumentCount; index += 2){
        // Current argument
        std::string arg = std::string(this->programArguments[index]);
        // Index of next argument in the array of arguments 
        int nextIndex = index + 1;

        // Server mode -> only option -l can be provided
        if(arg == "-l" && argumentCount == MIN_ARGUMENTS_COUNT){
            this->context = SERVER;
        }
        // Client mode -> option -r for set the hostname of the server
        else if(arg == "-r" && nextIndex < this->argumentCount && !file.empty()){
            this->context = CLIENT;
            file = std::string(this->programArguments[nextIndex]);
        }
        // Client mode -> option -s for set the file which will be send to the server
        else if(arg == "-s" && nextIndex < this->argumentCount && !hostname.empty()){
            this->context = CLIENT;
            hostname = std::string(this->programArguments[nextIndex]);
        }
        else{
            throw std::invalid_argument("Error: Invalid argument or argument was provided multiple times!");
        }
    }

    switch (this->context){
        case SERVER:{ 
            // TODO: Create server program instance
            break;
        }
        case CLIENT:{
            ipvSocketAddress ipvAddress;
            memset(&ipvAddress, 0, sizeof(ipvAddress));
            int ipVersion = this->convertHostname(hostname, ipvAddress);
            // TODO: Create client program instance
            break;
        }
        default:{ 
            throw std::runtime_error("Error: Internal problem in the ProgramContext class!");
            break;
        }
    }

}

int ProgramContext::convertHostname(std::string hostname, ipvSocketAddress &storedAddress){
    // Check if the hostname is empty -> internal error of the program, hostname should be parsed
    if(hostname.empty()){
        throw std::runtime_error("Error: Parsed hostname cannot be found!");
    }

    // Storage for converted ipv4
    struct in_addr ipv4Storage;
    // Storage for converted ipv6
    struct in6_addr ipv6Storage;

    // Try to convert the provided ipv4 address
    if(inet_pton(AF_INET, hostname.c_str(), &ipv4Storage) == 1){
        storedAddress.ipv4.sin_family = AF_INET;
        memcpy(&storedAddress.ipv4.sin_addr, &ipv4Storage, sizeof(struct in_addr));
        return AF_INET;
    }
    // Try to convert the provided ipv6 address
    else if(inet_pton(AF_INET6, hostname.c_str(), &ipv6Storage) == 1){
        storedAddress.ipv6.sin6_family = AF_INET6;
        memcpy(&storedAddress.ipv6.sin6_addr, &ipv6Storage, sizeof(struct in6_addr));
        return AF_INET6;
    }
   
    // Try to resolve the provided hostname
    struct addrinfo convertConfig, *convertList;
    memset(&convertConfig, 0, sizeof(convertConfig));
    memset(&convertList, 0, sizeof(convertList));

    // Configuration of the getaddrinfo function
    convertConfig.ai_family = AF_UNSPEC; 
    convertConfig.ai_socktype = SOCK_RAW;

    int returnCode = 0;
    returnCode = getaddrinfo(hostname.c_str(), nullptr, &convertConfig, &convertList);
    
    // Invalid hostname was provided
    if(returnCode == EAI_NONAME || returnCode == EAI_NODATA){
        throw std::invalid_argument("Error: Provided hostname is invalid!");
    }
    // Internal error of the getaddrinfo
    else if(returnCode != 0 || convertList == nullptr){
        throw std::runtime_error("Error: Internal error of getaddrinfo function!");
    }

    // OS will returned the best address in the first node of the linked list, so store the first ipv4 or ipv6 address
    if(convertList->ai_family == AF_INET){
        struct sockaddr_in *ipv4 = (struct sockaddr_in *)convertList->ai_addr;
        storedAddress.ipv4.sin_family = AF_INET;
        memcpy(&storedAddress.ipv4.sin_addr, &ipv4->sin_addr, sizeof(struct in_addr));
        freeaddrinfo(convertList);
        return AF_INET;
    }
    else if(convertList->ai_family == AF_INET6){
        struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)convertList->ai_addr;
        storedAddress.ipv6.sin6_family = AF_INET6;
        memcpy(&storedAddress.ipv6.sin6_addr, &ipv6->sin6_addr, sizeof(struct in6_addr));
        freeaddrinfo(convertList);
        return AF_INET6;
    }

    // Invalid argument was provided
    if(convertList != nullptr) freeaddrinfo(convertList);
    throw std::invalid_argument("Error: Provided hostname is not valid!");
}

std::unique_ptr<AbstractProgram>& ProgramContext::getProgram(){
    return this->program;
}