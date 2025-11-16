/** 
 * @file help.cpp
 * @author Martin Zůbek
 * @date 2025-10-25
 * @brief Source file with implementation of Help class. This class is derived from AbstractProgram class in [src|inc]/program.
 *        This context of program is used for displaying help information.
 */

#include <iostream>
#include "../../inc/program/help.hpp"

Help::Help(){}

void Help::run() {
    std::cout <<
        "Usage: secret -r <file> -s <ip|hostname> |  -l\n"
        "  -r <file>        File or filepath which contains the data to be sent.\n"
        "  -s <ip|hostname> Hostname or IP address of server.\n"
        "  -l               Listen mode/Server.\n";
}
