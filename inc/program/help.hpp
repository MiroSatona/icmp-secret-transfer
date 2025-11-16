/** 
 * @file help.hpp
 * @author Martin Zůbek
 * @date 2025-10-25
 * @brief Header file with definition of Help class. This class is derived from AbstractProgram class in [src|inc]/program.
 *        This context of program is used for displaying help information.
 */
#ifndef HELP_HPP
#define HELP_HPP

#include <iostream>
#include "abstract_program.hpp"

/**
 * @class Help
 * @brief Class for displaying help information.
 */
class Help: public AbstractProgram{
    public:
        /**
         * @brief Construct of the Help class
         * 
         */
        Help();
        /**
         * @brief Method for displaying help information.
         */
        void run() override;
};

#endif // HELP_HPP