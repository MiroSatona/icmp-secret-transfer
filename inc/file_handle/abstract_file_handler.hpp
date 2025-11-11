/**
 * @file abstract_file_handler.hpp
 * @author Martin Zůbek
 * @date 2025-10-10
 * @brief Header file with declaration of AbstractFileHandler class, which is an abstract class for file writer and reader classes.
 * 
 */
#ifndef ABSTRACT_FILE_HANDLER_HPP
#define ABSTRACT_FILE_HANDLER_HPP

#include <iostream>
#include <string>

// Maximum bytes size of part what can be read from the file
#define MAX_PART_SIZE_READ 1392

/**
 * @class AbstractFileHandler
 * @brief Abstract class for file writer and reader classes.
 */
class AbstractFileHandler{

    protected:
        /// @brief Name of the file which will be readed or written.
        std::string fileName;
        /**
         * @brief Method for parsing the file name form file path and storing it in the fileName attribute.
         * 
         * @param filePath File path provided from cmd line.
         */
        void parseFileNameFromPath(std::string filePath);

    public:
        /**
         * @brief Construct of the AbstractFileHandler class.
         */
        AbstractFileHandler();
        /**
         * @brief Virtual destructor of the AbstractFileHandler class.
         */
        virtual ~AbstractFileHandler() = default;
        /**
         * @brief Setter for the file name.
         * 
         * @param name Name of the file.
         */
        void setFileName(std::string name);
        /**
         * @brief Virtual method for closing the file stream.
         */
        virtual void closeStream() = 0;
        /**
         * @brief Getter for the file name.
         * 
         * @return std::string Name of the file.
         */
        std::string getFileName();

};

#endif // ABSTRACT_FILE_HANDLER_HPP