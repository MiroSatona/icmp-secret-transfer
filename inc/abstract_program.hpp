/**
 * @file abstract_program.hpp
 * @author Martin Zůbek
 * @date 2025-09-26
 * @brief Header file with definition of  AbstractProgram class. This class is an abstract base class for class Client in the client.[cpp|hpp] and class Serever in the server.[cpp|hpp].
 *        Wich class is instantiated independs on class ProgramContext in the program_context.[cpp|hpp] wich is reponsible for parsing command line arguments and deciding wich class will be instantiated.
 */

#ifndef ABSTRACT_PROGRAM_HPP
#define ABSTRACT_PROGRAM_HPP

/**
 * @class AbstractProgram
 * @brief Class define the interface for the class Client in the client.[cpp|hpp] and class Serever in the server.[cpp|hpp].
 */
class AbstractProgram {
    public:
        /**
         * @brief Virtual destructor.
         */
        virtual ~AbstractProgram() = default;
        /**
         * @brief Virtual method, wich will be implemented in the derived classes Client and Server.
         */
        virtual void run() const = 0;
};

#endif // ABSTRACT_PROGRAM_HPP