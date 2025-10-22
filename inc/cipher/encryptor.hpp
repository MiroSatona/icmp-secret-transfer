#ifndef ENCRYPTOR_HPP
#define ENCRYPTOR_HPP

#include <iostream>
#include <string>
#include "abstract_cipher.hpp"
#include "../types/struct_cipher.hpp"

class Encryptor : public AbstractCipher {
    public:
        Encryptor();
        CHIPHER_T encrypt(std::string plaintext) override;
};

#endif // ENCRYPTOR_HPP