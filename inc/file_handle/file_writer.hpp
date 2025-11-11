/**
 * @file file_writer.hpp
 * @author Martin Zůbek
 * @date 2025-10-10
 * @brief Header file with declaration of FileWriter class, which is responsible for writing data to a file.
 * 
 */
#ifndef FILE_WRITER_HPP
#define FILE_WRITER_HPP

#include <iostream>
#include <string>
#include <fstream>
#include "abstract_file_handler.hpp"

/**
 * @class FileWriter
 * @brief Class responsible for writing the data to a file.
 */
class FileWriter: public AbstractFileHandler {
    private:
        std::ofstream fileStream;
    public:
        /**
         * @brief Constructor of new FileWriter class.
         */
        FileWriter();
        /**
         * @brief Method for open the ofstream.
         */
        void openStream();
        /**
         * @brief Method for writing data to the file.
         * @param dataToWrite Data wich will be written.
         * @throw std::runtime_error If writing to file fails or if ofstream failed.
         */
         void write(std::string dataToWrite);
        /**
         * @brief Method for closing the ofstream.
         */
        void closeStream() override;
};

#endif // FILE_WRITER_HPP