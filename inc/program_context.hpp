
#ifndef PROGRAM_CONTEXT_HPP
#define PROGRAM_CONTEXT_HPP

#include <iostream>
#include <memory>
#include "abstract_program.hpp"

enum Context{
    CLIENT,
    SERVER
};


class ProgramContext{
private:
    std::unique_ptr<AbstractProgram> program;
    char **programArguments;
    int argumentCount;
    std::string filePath;
    Context context;

    void validateHostname(const std::string& hostname);
    std::string validateFilePath(const std::string& filePath);
public:
    ProgramContext(int argCount, char **argsArray);

    void getContext();
    std::unique_ptr<AbstractProgram>& getProgram();

};

#endif // PROGRAM_CONTEXT_HPP