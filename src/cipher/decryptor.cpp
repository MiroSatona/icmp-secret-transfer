/**
 * @file decryptor.cpp
 * @author Martin Zůbek
 * @date 2025-10-16
 * @brief File containing implementation of Decryptor class. There is implemented method for decrypting data using AES decryption and Constructor of the Decryptor class.
 */

#include <iostream>
#include <stdexcept>
#include <string.h>
#include <cstring>
#include <openssl/rand.h>
#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/aes.h>
#include "../../inc/cipher/decryptor.hpp"
#include "../../inc/types/struct_cipher.hpp"

// Constructor of the Decryptor class
Decryptor::Decryptor() : AbstractCipher() {}

/**
 * @note This implementation was inspired by the OpenSSL tutorial:
 *       https://wiki.openssl.org/index.php/EVP_Symmetric_Encryption_and_Decryption
 */
std::string Decryptor::decrypt(CHIPHER_T dataToDecrypt){

    // Create a context for the decryption
	EVP_CIPHER_CTX *cipherContext;
    memset(&cipherContext, 0, sizeof(cipherContext));
    if(!(cipherContext = EVP_CIPHER_CTX_new())){ 
        throw std::runtime_error("Error: Context for decryption failed!");
    }

    // Initialize the decryption operation
    if(EVP_DecryptInit_ex(cipherContext, EVP_aes_256_cbc(), NULL, this->key, dataToDecrypt.iv) != 1) {
        EVP_CIPHER_CTX_free(cipherContext);
        throw std::runtime_error("Error: Init of decryption failed!");
    }

    // Prepare storage for decrypted data
    unsigned char decryptedData[MAX_DATA_SIZE];
    memset(decryptedData, 0, MAX_DATA_SIZE);

    // Length of the decrypted output
    int outLengthDecrypt = 0;
    
    // Decrypt the data
    if(EVP_DecryptUpdate(cipherContext, decryptedData, &outLengthDecrypt, dataToDecrypt.data, dataToDecrypt.totalLen) != 1) {
        EVP_CIPHER_CTX_free(cipherContext);
        throw std::runtime_error("Error: Decryption update failed!");
    }

    // Length of the final block
    int finalBlockLength = 0;
    
    // Decrypt the final block
    if(EVP_DecryptFinal_ex(cipherContext, decryptedData + outLengthDecrypt, &finalBlockLength) != 1) {
        EVP_CIPHER_CTX_free(cipherContext);
        throw std::runtime_error("Error: Final decryption step failed!");
    }

    // Calculate total length of decrypted data
    int totalLen = outLengthDecrypt + finalBlockLength;

    // Clean up and return decrypted data
    EVP_CIPHER_CTX_free(cipherContext);
    return std::string((char*)decryptedData, totalLen);
}

