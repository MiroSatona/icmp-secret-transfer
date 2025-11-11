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
    if(this->fileStream.is_open()) return;
    this->fileStream.open(this->fileName, std::ios::out |  std::ios::binary);
    if(!this->fileStream.is_open()){
        throw std::runtime_error("Error: Open of fstream failed!");
    }
}

void FileWriter::write(std::string dataToWrite) {
    if (!this->fileStream.is_open()) {
        throw std::runtime_error("Error: File stream failed!");
    }

    this->fileStream.write(dataToWrite.c_str(), dataToWrite.size());

    if (!this->fileStream.good()) {
        throw std::runtime_error("Error: File stream failed!");
    }
}

void FileWriter::closeStream(){
    if(this->fileStream.is_open()) this->fileStream.close();
}