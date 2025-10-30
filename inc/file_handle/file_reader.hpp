/**
 * @file file_reader.hpp
 * @author Martin Zůbek
 * @date 2025-10-10
 * @brief Header file with declaration of FileReader class, which is responsible for reading data from a file.
 */
#ifndef FILE_READER_HPP
#define FILE_READER_HPP

#include <iostream>
#include <string>
#include <fstream>
#include "abstract_file_handler.hpp"

/**
 * @class FileReader
 * @brief Class responsible for reading data from a file.
 */
class FileReader: public AbstractFileHandler {
   private:
        /// @brief Path of the file which was provided from cmd line.
        std::string filePath;
        /// @brief Input file stream for reading data from the file.
        std::ifstream fileStream;
    public:
        /**
         * @brief Construct a new File Reader object
         * 
         * @param filePath Path of the file which will be read. 
         */
        FileReader(std::string filePath);
        /**
         * @brief Method for reading data from the file.
         * 
         * @throw std::runtime_error If reading of file fails or if ifstream failed.
         * 
         * @return unsigned char* Readed data from the file.
         */
        unsigned char* read() override;
        /**
         * @brief Method for closing the ifstream.
         */
        void closeStream() override;
};

#endif // FILE_READER_HPP