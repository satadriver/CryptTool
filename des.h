#pragma once

#include <stdint.h>


extern uint8_t* des_decrypt(const uint8_t* ciphertext, size_t ciphertext_len, const uint8_t* key, size_t* plaintext_len);

extern uint8_t* des_encrypt(const uint8_t* plaintext, size_t plaintext_len, const uint8_t* key, size_t* ciphertext_len);
