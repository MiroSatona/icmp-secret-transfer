/**
 * @file abstract_program.hpp
 * @author Martin Zůbek
 * @date 2025-09-26
 * @brief Header file with definition of  AbstractProgram class. This class is an abstract base class for class Client in the [src|inc]/program/client.[cpp|hpp] and class Serever in the [src|inc]/program/server.[cpp|hpp].
 *        Wich class is instantiated independs on class ProgramContext in the [src|inc]/program/program_context.[cpp|hpp] wich is reponsible for parsing command line arguments and deciding wich class will be instantiated.
 */

#ifndef ABSTRACT_PROGRAM_HPP
#define ABSTRACT_PROGRAM_HPP

#include <iostream>
/**
 * @class AbstractProgram
 * @brief Class define the abstract class for the class Client in the [src|inc]/program/client.[cpp|hpp] and class Serever in the [src|inc]/program/server.[cpp|hpp].
 */
class AbstractProgram {
    public:
        /**
         * @brief Default constructor of AbstractProgram.
         */
        AbstractProgram() = default;       
        /**
         * @brief Virtual destructor.
         */
        virtual ~AbstractProgram() = default;
        /**
         * @brief Virtual method, wich will be implemented in the derived classes Client and Server.
         */
        virtual void run() = 0;
};

#endif // ABSTRACT_PROGRAM_HPP