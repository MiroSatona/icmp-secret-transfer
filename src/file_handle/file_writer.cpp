 /**  
 * @file file_writer.hpp
 * @author Martin Zůbek
 * @date 2025-10-10
 * @brief Implementation file of methods of FileWriter class, which is responsible for writing decrypted data to a file.
 */

#include <iostream>
#include <ios>
#include <string>
#include <fstream>
#include "../../inc/file_handle/file_writer.hpp"
#include "../../inc/file_handle/abstract_file_handler.hpp"

FileWriter::FileWriter() : AbstractFileHandler() {}

void FileWriter::openStream(){
    // Check if stream is opened
    if(this->fileStream.is_open()) return;
    // Else open stream
    this->fileStream.open(this->fileName, std::ios::out |  std::ios::binary);
    // Check if opening the stream was successful
    if(!this->fileStream.is_open()){
        throw std::runtime_error("Error: Open of fstream failed!");
    }
}

void FileWriter::write(std::string dataToWrite) {
    // Check if stream for writing is open
    if (!this->fileStream.is_open()) {
        throw std::runtime_error("Error: File stream failed!");
    }
    // Write data to the file
    this->fileStream.write(dataToWrite.c_str(), dataToWrite.size());

    // Check if the write operation was successful
    if (!this->fileStream.good()) {
        throw std::runtime_error("Error: File stream failed!");
    }
}

void FileWriter::closeStream(){
    if(this->fileStream.is_open()) this->fileStream.close();
}