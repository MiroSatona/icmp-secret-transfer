/**
 * @file abstract_file_handler.cpp
 * @author Martin Zůbek
 * @date 2025-10-10
 * @brief Implementation file of methods of AbstractFileHandler class, wich are used in file writer and reader classes.
 */

#include <iostream>
#include <string>
#include <stdexcept>
#include <ios>
#include <filesystem>
#include <fstream>
#include <cstring>
#include "../../inc/file_handle/file_reader.hpp"

FileReader::FileReader(std::string filePath) : AbstractFileHandler() {
    // Save the file path or name of file which will be read
    this->filePath = filePath;
    
    // Check if the file exists
    if (!std::filesystem::exists(this->filePath)) {
        throw std::invalid_argument("Error: Provided file does not exist!");
    }
    // Check if the file is a regular file
    else if (!std::filesystem::is_regular_file(this->filePath)) {
        throw std::invalid_argument("Error: Provided file is not a file!");
    }

    // Try to open the file for binary mode reading
    this->fileStream.open(this->filePath, std::ios::binary);

    // Check if the file can be read
    if (!this->fileStream.is_open()) {
        throw std::invalid_argument("Error: Unopenable file was provided!");
    }

    // Get name of the file from the provided path
    this->parseFileNameFromPath(this->filePath);

}


unsigned char *FileReader::read(){
    // Check if the file stream is open
    if (!fileStream.is_open()) {
        throw std::runtime_error("Error: File stream failed!");
    }

    // Buffer for storing readed part from the file
    char buffer[MAX_PART_SIZE_READ];
    // Read part of the file
    this->fileStream.read(buffer, MAX_PART_SIZE_READ);

    // Get count of readed bytes
    std::streamsize bytesRead = this->fileStream.gcount();

    // End of file was reached
    if (bytesRead == 0 && this->fileStream.eof()) {
        return nullptr; 
    }
    // Runtime error of reading the file
    else if (bytesRead < 0) {
        throw std::runtime_error("Error: Reading of the file failed!");
    }
    // Return the readed part from the file with right size
    unsigned char *readedPart = new unsigned char[bytesRead];
    memcpy(readedPart, (unsigned char*)buffer, bytesRead);
    return readedPart;
}

void FileReader::closeStream(){
    if(this->fileStream.is_open()) this->fileStream.close();
}