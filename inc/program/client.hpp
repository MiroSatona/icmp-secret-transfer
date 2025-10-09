/**
 * @file client.hpp
 * @author Martin Zůbek
 * @date 2025-10-09
 * @brief Header file containing the declaration of the Client class. This class is responsible for handling states of the protocol.
 *        Independent on the state, TODO: Finish description after implementation of the and conception of the program will be done.
 */
#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <iostream>
#include <string>
#include "abstract_program.hpp"
#include "../types/union_address.hpp"

class Client : public AbstractProgram{
    private:
        int state;
    public:
        Client(int ipVer, ipvSocketAddress serverAddress, std::string filePath);
        void run() const override;
};

#endif // CLIENT_HPP

