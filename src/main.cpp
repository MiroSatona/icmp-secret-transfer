/**
* @file main.cpp
* @author Martin Zůbek
* @date 2025-09-26
* @brief Main file of the project. This file contains the main function wich is the entry point of the program and handle the program flow.
*/

#include <iostream>
#include <stdexcept>
#include "../inc/program/program_context.hpp"
#include "../inc/types/return_values.hpp"

/**
 * @brief The main function what is the entry point of the program.
 * @param argc Count of command line arguments.
 * @param argv Array of arguments what was passed while starting the program from command line.
 * @return Return value of the program, wich is defined in the inc/types/return_values.hpp.
 *         ReturnValues::Ok = 0, if everything is ok.
 *         ReturnValues::ARGUMENT_ERROR = 1, if any argument is invalid or if invalid combination of arguments is provided.
 *         ReturnValues::PROTOCOL_ERROR = 2, if any error of the protocol was reached.
 *         ReturnValues::INTERNAL_ERROR = 3, if any internal error of the program was reached.
 */
int main(int argc, char** argv) {
    
    try{
        // Instantiate the program context
        ProgramContext context = ProgramContext(argc, argv);
        // Get the context of the program by method getContext whats independent on the provided arguments form cmd line
        context.getContext();
        // Get the program instance, the instance will be client or server instance
        std::unique_ptr<AbstractProgram>& program = context.getProgram();
        // Run the program
        program->run();

    }catch(const std::invalid_argument& argumentException){
        std::cerr << argumentException.what() << std::endl;
        return ReturnValues::ARGUMENT_ERROR;
    }catch(const std::logic_error& protocolException){
        std::cerr << protocolException.what() << std::endl;
        return ReturnValues::PROTOCOL_ERROR;
    }catch(const std::runtime_error& internalException){
        std::cerr << internalException.what() << std::endl;
        return ReturnValues::INTERNAL_ERROR;
    }

    return ReturnValues::OK;    
}