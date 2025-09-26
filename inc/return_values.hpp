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
    OK,                // Successful execution of the program
    ARGUMENT_ERROR,    // Invalid command line arguments was passed or one of the arguments are invalid
    PROTOCOL_ERROR,    // Error of the protocol
    INTERNAL_ERROR,    // Internal error of the program
};

#endif // RETURN_VALUES_HPP
