/**
 * @file abstract_cipher.hpp
 * @author Martin Zůbek
 * @date 2025-10-16
 * @brief Header file with definition of AbstractCipher class. This class is an abstract class for 
 * Encryptor in the [src|inc]/cipher/encryptor.[cpp|hpp] and Decryptor in the [src|inc]/cipher/decryptor.[cpp|hpp].
 */
#ifndef ABSTRACT_CIPHER_HPP
#define ABSTRACT_CIPHER_HPP

#include <iostream>
#include <string>
#include "../types/struct_cipher.hpp"

// Encryption/decryption key
#define LOGIN_KEY "xlogin0"
// Size of the encryption/decryption key
#define LOGIN_KEY_SIZE 7
// Size of the hashed key
#define KEY_SIZE_BYTES 32
/**
 * @class AbstractCipher
 * @brief Abstract class for encryption class in the [src|inc]/cipher/encryptor.[cpp|hpp] and decryption  class in the [src|inc]/cipher/decryptor.[cpp|hpp].
 */
class AbstractCipher {
    protected:
        /// @brief The hashed encryption/decryption key.
        unsigned char key[KEY_SIZE_BYTES];
    public:
        /**
         * @brief Constructor of AbstractCipher class used for hash the key.
         */
        AbstractCipher();
        /**
         * @brief Virtual destructor.
         */
        virtual ~AbstractCipher() = default;
};

#endif // ABSTRACT_CIPHER_HPP

