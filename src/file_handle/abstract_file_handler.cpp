/**
 * @file abstract_file_handler.cpp
 * @author Martin Zůbek
 * @date 2025-10-10
 * @brief Implementation file of methods of AbstractFileHandler class, wich are used in file writer and reader classes.
 */
#include <iostream>
#include <string>
#include <filesystem>
#include "../../inc/file_handle/abstract_file_handler.hpp"

AbstractFileHandler::AbstractFileHandler() : fileName("") {}

void AbstractFileHandler::parseFileNameFromPath(std::string filePath){
    // Parsing the file name from the provided file path
    std::filesystem::path pathObj(filePath);
    this->fileName = pathObj.filename().string();
}

std::string AbstractFileHandler::getFileName(){
    return this->fileName;
}

void AbstractFileHandler::setFileName(std::string name){
    this->fileName = name;
}