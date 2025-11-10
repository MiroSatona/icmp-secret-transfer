/**
 * @file encryptor.cpp
 * @author Martin Zůbek
 * @date 2025-10-16
 * @brief File containing implementation of Encryptor class. There is implemented method for encrypting data using AES encryption and Constructor of the Encryptor class.
 */
#include <iostream>
#include <vector>
#include <string.h>
#include <cstring>
#include <openssl/rand.h>
#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/aes.h>
#include "../../inc/cipher/encryptor.hpp"
#include "../../inc/types/struct_cipher.hpp"


// Constructor of the Encryptor class
Encryptor::Encryptor() : InterfaceEncryptor() {}

CHIPHER_T Encryptor::encrypt(unsigned char *dataToEncrypt) {

    // Prepate the CHIPHER_T struct for storing the result and metadata
    CHIPHER_T cipherResult;
    memset(&cipherResult, 0, sizeof(CHIPHER_T));

    // Generate a random initialization vector and save him to the CHIPHER_T struct
    unsigned char iv[AES_BLOCK_SIZE];
    memset(iv, 0, AES_BLOCK_SIZE);

    if (!RAND_bytes(iv, AES_BLOCK_SIZE)){ 
        throw std::runtime_error("IV generation failed");
    }

    memcpy(cipherResult.iv, iv, AES_BLOCK_SIZE);

    // Create a context for the encryption
    EVP_CIPHER_CTX *cipherContext;
    memset(&cipherContext, 0, sizeof(cipherContext));

    if(!(cipherContext = EVP_CIPHER_CTX_new())){ 
        throw std::runtime_error("Error: Context for encryption failed!");
    }

    // Initialize the encryption operation
    if(EVP_EncryptInit_ex(cipherContext, EVP_aes_256_cbc(), NULL, this->key, iv) != 1) {
        EVP_CIPHER_CTX_free(cipherContext);
        throw std::runtime_error("Error: Init of encryption failed!");
    }

    // Prepare storage for encrypted data
    unsigned char cipherData[MAX_DATA_SIZE];
    memset(cipherData, 0, MAX_DATA_SIZE);

    // Length of the output cipher data
    int outLengthCipher;
    memset(&outLengthCipher, 0, sizeof(outLengthCipher));

    // Length of the input data
    int inputLengthData = strlen((char*)dataToEncrypt);

    if(EVP_EncryptUpdate(cipherContext, cipherData, &outLengthCipher, dataToEncrypt, inputLengthData) != 1) {
        EVP_CIPHER_CTX_free(cipherContext);
        throw std::runtime_error("Error: Encryption update failed!");
    }

    // Length of final block
    int finalBlockLength;
    memset(&finalBlockLength, 0, sizeof(finalBlockLength));
    // Encrypt the final block which will have padding
    if(EVP_EncryptFinal_ex(cipherContext, cipherData + outLengthCipher, &finalBlockLength) != 1) {
        EVP_CIPHER_CTX_free(cipherContext);
        throw std::runtime_error("Error: Final encryption step failed!");
    }

    // Get the total length
    cipherResult.totalLen = outLengthCipher + finalBlockLength;

    // Copy the encrypted data to the CHIPHER_T struct and clean up
    memcpy(cipherResult.data, cipherData, cipherResult.totalLen);
    EVP_CIPHER_CTX_free(cipherContext);
    return cipherResult;
}