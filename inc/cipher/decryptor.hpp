/**
 * @file decryptor.hpp
 * @author Martin Zůbek
 * @date 2025-11-01
 * @brief Header file with definition of Decryptor class. This class is responsible for decrypting data.
 */

#ifndef DECRYPTOR_HPP
#define DECRYPTOR_HPP

#include <iostream>
#include <string>
#include "abstract_cipher.hpp"
#include "../types/struct_cipher.hpp"

/**
 * @class Decryptor
 * @brief Class responsible for decrypting data using AES decryption.
 */
class Decryptor : public InterfaceDecryptor {
    public:
        /**
         * @brief Constructor of Decryptor class.
         */
        Decryptor();
        /**
         * @brief Method for decrypt the provided data.
         * @param dataToDecrypt Data which will be decrypted.
         * @throw std::runtime_error If any error was reached during the decryption.
         * @return std::string Decrypted data.
         */
        std::string decrypt(CHIPHER_T dataToDecrypt) override;
};

#endif // DECRYPTOR_HPP