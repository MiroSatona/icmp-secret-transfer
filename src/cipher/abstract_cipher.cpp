/**
 * @file abstract_cipher.cpp
 * @author Martin Zůbek
 * @date 2025-10-16
 * @brief Source file with implementation of constructor of AbstractCipher class for hashing the encryption/decryption key.
 */
#include <cstring>
#include <openssl/sha.h>
#include "../../inc/cipher/abstract_cipher.hpp"

AbstractCipher::AbstractCipher() { SHA256((const unsigned char*)LOGIN_KEY, LOGIN_KEY_SIZE, this->key);}