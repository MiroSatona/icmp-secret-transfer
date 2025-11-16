/**
 * @file program_context.hpp
 * @author Martin Zůbek
 * @date 2025-09-26
 * @brief Header file with definition of ProgramContext class. This class is responsible for parsing cmd line arguments
 *        and decides wich class, for representing the program will be instantiated, Client or Server or Help.
 */ 

#ifndef PROGRAM_CONTEXT_HPP
#define PROGRAM_CONTEXT_HPP

#include <iostream>
#include <memory>
#include <variant>
#include <arpa/inet.h>
#include <netdb.h>
#include "abstract_program.hpp"

// The minimum count of the provided arguments from cmd line.
#define MIN_ARGUMENTS_COUNT 2

/**
 * @enum Context
 * @brief Enumeration for represents the context of the program, wich is get while the cmd line arguments are parsed. 
 */
enum Context{
    CLIENT,
    SERVER,
    HELP,
    NOT_SET
};

/**
 * @class ProgramContext
 * @brief Class wich is parsing the cmd line arguments and decides wich class, for representing the program will be instantiated, Client or Server.
 *        The instace is stored in the private attribute of the ProgramContext class as a unique pointer of the AbstractProgram class.
 */
class ProgramContext{

private:
    /// @brief The instance of the program, Client from the src/program/client.[cpp|hpp] or Server from the src/program/server.[cpp|hpp] or Help from the src/program/help.[cpp|hpp].
    std::unique_ptr<AbstractProgram> program;
    /// @brief Array of arguments wich was pasted in the cmd line.
    char **programArguments;
    /// @brief Count of the arguments wich was pasted in the cmd line.
    int argumentCount;
    /// @brief The context of the program, wich is get while the cmd line arguments are parsed.
    Context context;

    /**
     * @brief Method for converting the provided ipv4 or ipv6 address or hostname to the sockaddr_in or sockaddr_in6 structure from sys/socket.h and netinet/in.h.
     * @param hostname The provided hostname or ipv4 or ipv6 address.
     * @param storedAddress The reference to the unique variant for storing either sockaddr_in or sockaddr_in6 structure.
     * @throw std::invalid_argument If the provided hostname or ipv address is invalid.
     * @throw std::runtime_error If any internal error was reached.
     * @return int Return AF_INET or AF_INET6 from <arpa/inet.h> depending on the type of the converted address.
     */
    int convertHostname(std::string hostname, std::variant<struct sockaddr_in, struct sockaddr_in6> &storedAddress);

public:
    /**
     * @brief Constructor of the ProgramContext class.
     * @param argCount Count of the arguments wich was pasted in the cmd line.
     * @param argsArray Array of arguments wich was pasted in the cmd line.
     */
    ProgramContext(int argCount, char **argsArray);
    /**
     * @brief Method for parse the cmd line arguments and set the context of the program.
     * @throw std::invalid_argument If any argument from cmd line is invalid or if is provided invalid combination of arguments.
     */
    void getContext();
    /**
     * @brief Getter for the instance of the program, Client from the src/program/client_ipv[4|6].[cpp|hpp] or Server from the src/program/server.[cpp|hpp].
     * @return The reference to the unique pointer of the AbstractProgram class.
     */
    std::unique_ptr<AbstractProgram>& getProgram();

};

#endif // PROGRAM_CONTEXT_HPP