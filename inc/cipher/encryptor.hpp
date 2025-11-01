/**
 * @file encryptor.hpp
 * @author Martin Zůbek
 * @date 2025-10-16
 * @brief Header file with definition of Encryptor class. This class is responsible for encrypting data.
 */

#ifndef ENCRYPTOR_HPP
#define ENCRYPTOR_HPP

#include <iostream>
#include <string>
#include "abstract_cipher.hpp"
#include "../types/struct_cipher.hpp"

/**
 * @class Encryptor
 * @brief Class responsible for encrypting data using AES encryption.
 */
class Encryptor : public AbstractCipher {
    public:
        /**
         * @brief Constructor of Enryptor class.
         */
        Encryptor();
        /**
         * @brief Method for encrypt the provided data.
         * @param dataToEncrypt Data which will be encrypted.
         * @throw std::runtime_error If any error was reached during the encryption.
         * @return CHIPHER_T struct with metadata(padding,..) and cipher data. See in inc/types/struct_cipher.hpp.
         */
        CHIPHER_T encrypt(unsigned char *dataToEncrypt) override;
};

#endif // ENCRYPTOR_HPP