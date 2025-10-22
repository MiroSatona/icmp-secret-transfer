#ifndef ABSTRACT_CIPHER_HPP
#define ABSTRACT_CIPHER_HPP

#include <iostream>
#include <string>
#include "abstract_file_handler.hpp"
#include "../types/struct_cipher.hpp"

#define LOGIN_KEY "xlogin0"
#define LOGIN_KEY_SIZE 8
#define KEY_SIZE_BYTES 32


class AbstractCipher {
    protected:
        unsigned char key[KEY_SIZE_BYTES];
    public:
        AbstractCipher();
        virtual CHIPHER_T encrypt(std::string plaintext) = 0;
        virtual std::string decrypt(std::string ciphertext) = 0;
        virtual ~AbstractCipher() = default;
};

#endif // ABSTRACT_CIPHER_HPP

