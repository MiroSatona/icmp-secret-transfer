/**
 * @file program_context.cpp
 * @author Martin Zůbek
 * @date 2025-09-26
 * @brief Source file with implementation of ProgramContext class. This class is responsible for parsing cmd line arguments and decides wich class, for representing the program will be instantiated, Client or Server.
 */
#include <iostream>
#include <stdexcept>
#include <arpa/inet.h>
#include <netdb.h>
#include <cstring>
#include <memory>
#include <variant>
#include "../../inc/program/abstract_program.hpp"
#include "../../inc/program/program_context.hpp"
#include "../../inc/program/client.hpp"

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
        // Client mode -> option -s for set the file which will be send to the server
        else if(arg == "-s" && nextIndex < this->argumentCount && hostname.empty()){
            this->context = CLIENT;
            hostname = std::string(this->programArguments[nextIndex]);
        }
        // Client mode -> option -r for set the hostname of the server
        else if(arg == "-r" && nextIndex < this->argumentCount && file.empty()){
            this->context = CLIENT;
            file = std::string(this->programArguments[nextIndex]);
        }
        else{
            throw std::invalid_argument("Error: Invalid argument or argument was provided multiple times!");
        }
    }

    std::variant<struct sockaddr_in, struct sockaddr_in6> ipvAddress;
    int typeOfAddress = this->convertHostname(hostname, ipvAddress);

    switch (this->context){
        case SERVER:{
            break;
        }
        case CLIENT:{
            if(typeOfAddress == AF_INET){
                struct sockaddr_in serverAddr = std::get<struct sockaddr_in>(ipvAddress);
                this->program = std::make_unique<Client>(serverAddr, file, typeOfAddress);
            } else if(typeOfAddress == AF_INET6){
                struct sockaddr_in6 serverAddr = std::get<struct sockaddr_in6>(ipvAddress);
                this->program = std::make_unique<Client>(serverAddr, file, typeOfAddress);
            }
            break;
        }
        default:{ 
            throw std::runtime_error("Error: Internal problem in the ProgramContext class!");
            break;
        }
    }
}

int ProgramContext::convertHostname(std::string hostname, std::variant<struct sockaddr_in, struct sockaddr_in6> &storedAddress){
    if(hostname.empty()){
        throw std::runtime_error("Error: Parsed hostname cannot be found!");
    }

    // Storages for converted addresses
    struct in_addr ipv4Storage;
    struct in6_addr ipv6Storage;

    // Check if the provided hostname is ipv4 or ipv6 address
    if(inet_pton(AF_INET, hostname.c_str(), &ipv4Storage) == 1){
        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        memcpy(&addr.sin_addr, &ipv4Storage, sizeof(struct in_addr));
        storedAddress = addr;
        this->context = CLIENT;
        return AF_INET;
    }
    else if(inet_pton(AF_INET6, hostname.c_str(), &ipv6Storage) == 1){
        struct sockaddr_in6 addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin6_family = AF_INET6;
        memcpy(&addr.sin6_addr, &ipv6Storage, sizeof(struct in6_addr));
        storedAddress = addr;
        this->context = CLIENT;
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
        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        memcpy(&addr.sin_addr, &ipv4->sin_addr, sizeof(struct in_addr));
        storedAddress = addr;
        freeaddrinfo(convertList);
        this->context = CLIENT;
        return AF_INET;
    }
    else if(convertList->ai_family == AF_INET6){
        struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)convertList->ai_addr;
        struct sockaddr_in6 addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin6_family = AF_INET6;
        memcpy(&addr.sin6_addr, &ipv6->sin6_addr, sizeof(struct in6_addr));
        storedAddress = addr;
        freeaddrinfo(convertList);
        this->context = CLIENT;
        return AF_INET6;
    }

    // Invalid argument was provided
    if(convertList != nullptr) freeaddrinfo(convertList);
    throw std::invalid_argument("Error: Provided hostname is not valid!");
}

std::unique_ptr<AbstractProgram>& ProgramContext::getProgram(){
    return this->program;
}