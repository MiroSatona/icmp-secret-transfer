

#include "../inc/program_context.hpp"

ProgramContext::ProgramContext(int argCount, char **argsArray) : argumentCount(argCount), programArguments(argsArray) {}


void ProgramContext::getContext(){
    if(this->argumentCount < 2) {
        throw std::invalid_argument("Error: Any argument was not provided!");
    }
    bool file = false;
    bool hostname = false;

    for(int index = 1; index < this->argumentCount; index += 2){
        // Current argument
        std::string arg = std::string(this->programArguments[index]);
        // Index of next argument in the array of arguments
        int nextIndex = index + 1;

        if(arg == "-l" && argumentCount == 2){
            this->context = SERVER;
        }
        else if(arg == "-r" && nextIndex < this->argumentCount && !file){
            this->context = CLIENT;
            file = true;
            this->filePath = validateFilePath(std::string(this->programArguments[nextIndex]));
            
        }
        else if(arg == "-r" && nextIndex < this->argumentCount && !hostname){
            this->context = CLIENT;
            hostname = true;
            // TODO: Validate and store hostname
        }
        else{
            throw std::invalid_argument("Error: Invalid argument or argument was provided multiple times!");
        }
    }

    switch (this->context)
    {
    case SERVER:
        // TODO: Create server program instance
        break;
    case CLIENT:
        // TODO: Create client program instance
        break;
    }

}