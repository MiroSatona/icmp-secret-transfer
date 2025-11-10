/**
 * @file struct_cipher.hpp
 * @author Martin Zůbek
 * @date 2025-10-13
 * @brief Header file with definition of struct CHIPHER_T.
 */

#ifndef STRUCT_CIPHER_HPP
#define STRUCT_CIPHER_HPP


// Maximum size of data to send
#define  MAX_DATA_SIZE 1376

#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <openssl/aes.h>

/**
 * @struct CHIPHER_T
 * @brief Structure representing a cipher data with initialization vector and total length.
 */
struct CHIPHER_T
{
    /// @brief The data.
    unsigned char data[MAX_DATA_SIZE];
    /// @brief Initialization vector used in encryption/decryption.
    unsigned char iv[AES_BLOCK_SIZE];
    /// @brief Total length of the cipher data.
    int totalLen;
};

#endif // STRUCT_CIPHER_HPP

