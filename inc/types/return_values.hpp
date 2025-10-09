/**
 * @file return_values.hpp
 * @author Martin Zůbek
 * @date 2025-09-26
 * @brief Header file with definition of enumeration of return values of the program.
 */

#ifndef RETURN_VALUES_HPP
#define RETURN_VALUES_HPP

/**
 * @enum ReturnValues
 * @brief Enumeration of return values for the program.
 */
enum ReturnValues{
    OK = 0,                // Successful execution of the program
    ARGUMENT_ERROR = 1,    // Invalid command line arguments was passed or one of the arguments are invalid
    PROTOCOL_ERROR = 2,    // Error of the protocol
    INTERNAL_ERROR = 3,    // Internal error of the program
};

#endif // RETURN_VALUES_HPP
