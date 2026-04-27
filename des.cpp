#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// ==================== DES 常量表 ====================

// 初始置换 IP
static const uint8_t IP[64] = {
    58, 50, 42, 34, 26, 18, 10, 2,
    60, 52, 44, 36, 28, 20, 12, 4,
    62, 54, 46, 38, 30, 22, 14, 6,
    64, 56, 48, 40, 32, 24, 16, 8,
    57, 49, 41, 33, 25, 17, 9,  1,
    59, 51, 43, 35, 27, 19, 11, 3,
    61, 53, 45, 37, 29, 21, 13, 5,
    63, 55, 47, 39, 31, 23, 15, 7
};

// 逆初始置换 IP^-1
static const uint8_t FP[64] = {
    40, 8, 48, 16, 56, 24, 64, 32,
    39, 7, 47, 15, 55, 23, 63, 31,
    38, 6, 46, 14, 54, 22, 62, 30,
    37, 5, 45, 13, 53, 21, 61, 29,
    36, 4, 44, 12, 52, 20, 60, 28,
    35, 3, 43, 11, 51, 19, 59, 27,
    34, 2, 42, 10, 50, 18, 58, 26,
    33, 1, 41, 9,  49, 17, 57, 25
};

// 扩展置换 E (32bit -> 48bit)
static const uint8_t E[48] = {
    32, 1,  2,  3,  4,  5,
    4,  5,  6,  7,  8,  9,
    8,  9,  10, 11, 12, 13,
    12, 13, 14, 15, 16, 17,
    16, 17, 18, 19, 20, 21,
    20, 21, 22, 23, 24, 25,
    24, 25, 26, 27, 28, 29,
    28, 29, 30, 31, 32, 1
};

// S 盒 (8个，每个 4x16)
static const uint8_t S[8][4][16] = { {
    { 14, 4,  13, 1,  2,  15, 11, 8,  3,  10, 6,  12, 5,  9,  0,  7 },
    { 0,  15, 7,  4,  14, 2,  13, 1,  10, 6,  12, 11, 9,  5,  3,  8 },
    { 4,  1,  14, 8,  13, 6,  2,  11, 15, 12, 9,  7,  3,  10, 5,  0 },
    { 15, 12, 8,  2,  4,  9,  1,  7,  5,  11, 3,  14, 10, 0,  6,  13 }
}, {
    { 15, 1,  8,  14, 6,  11, 3,  4,  9,  7,  2,  13, 12, 0,  5,  10 },
    { 3,  13, 4,  7,  15, 2,  8,  14, 12, 0,  1,  10, 6,  9,  11, 5 },
    { 0,  14, 7,  11, 10, 4,  13, 1,  5,  8,  12, 6,  9,  3,  2,  15 },
    { 13, 8,  10, 1,  3,  15, 4,  2,  11, 6,  7,  12, 0,  5,  14, 9 }
}, {
    { 10, 0,  9,  14, 6,  3,  15, 5,  1,  13, 12, 7,  11, 4,  2,  8 },
    { 13, 7,  0,  9,  3,  4,  6,  10, 2,  8,  5,  14, 12, 11, 15, 1 },
    { 13, 6,  4,  9,  8,  15, 3,  0,  11, 1,  2,  12, 5,  10, 14, 7 },
    { 1,  10, 13, 0,  6,  9,  8,  7,  4,  15, 14, 3,  11, 5,  2,  12 }
}, {
    { 7,  13, 14, 3,  0,  6,  9,  10, 1,  2,  8,  5,  11, 12, 4,  15 },
    { 13, 8,  11, 5,  6,  15, 0,  3,  4,  7,  2,  12, 1,  10, 14, 9 },
    { 10, 6,  9,  0,  12, 11, 7,  13, 15, 1,  3,  14, 5,  2,  8,  4 },
    { 3,  15, 0,  6,  10, 1,  13, 8,  9,  4,  5,  11, 12, 7,  2,  14 }
}, {
    { 2,  12, 4,  1,  7,  10, 11, 6,  8,  5,  3,  15, 13, 0,  14, 9 },
    { 14, 11, 2,  12, 4,  7,  13, 1,  5,  0,  15, 10, 3,  9,  8,  6 },
    { 4,  2,  1,  11, 10, 13, 7,  8,  15, 9,  12, 5,  6,  3,  0,  14 },
    { 11, 8,  12, 7,  1,  14, 2,  13, 6,  15, 0,  9,  10, 4,  5,  3 }
}, {
    { 12, 1,  10, 15, 9,  2,  6,  8,  0,  13, 3,  4,  14, 7,  5,  11 },
    { 10, 15, 4,  2,  7,  12, 9,  5,  6,  1,  13, 14, 0,  11, 3,  8 },
    { 9,  14, 15, 5,  2,  8,  12, 3,  7,  0,  4,  10, 1,  13, 11, 6 },
    { 4,  3,  2,  12, 9,  5,  15, 10, 11, 14, 1,  7,  6,  0,  8,  13 }
}, {
    { 4,  11, 2,  14, 15, 0,  8,  13, 3,  12, 9,  7,  5,  10, 6,  1 },
    { 13, 0,  11, 7,  4,  9,  1,  10, 14, 3,  5,  12, 2,  15, 8,  6 },
    { 1,  4,  11, 13, 12, 3,  7,  14, 10, 15, 6,  8,  0,  5,  9,  2 },
    { 6,  11, 13, 8,  1,  4,  10, 7,  9,  5,  0,  15, 14, 2,  3,  12 }
}, {
    { 13, 2,  8,  4,  6,  15, 11, 1,  10, 9,  3,  14, 5,  0,  12, 7 },
    { 1,  15, 13, 8,  10, 3,  7,  4,  12, 5,  6,  11, 0,  14, 9,  2 },
    { 7,  11, 4,  1,  9,  12, 14, 2,  0,  6,  10, 13, 15, 3,  5,  8 },
    { 2,  1,  14, 7,  4,  10, 8,  13, 15, 12, 9,  0,  3,  5,  6,  11 }
} };

// P 置换 (32bit)
static const uint8_t P[32] = {
    16, 7, 20, 21, 29, 12, 28, 17,
    1,  15, 23, 26, 5,  18, 31, 10,
    2,  8,  24, 14, 32, 27, 3,  9,
    19, 13, 30, 6,  22, 11, 4,  25
};

// PC-1 (64bit -> 56bit)
static const uint8_t PC1[56] = {
    57, 49, 41, 33, 25, 17, 9,
    1,  58, 50, 42, 34, 26, 18,
    10, 2,  59, 51, 43, 35, 27,
    19, 11, 3,  60, 52, 44, 36,
    63, 55, 47, 39, 31, 23, 15,
    7,  62, 54, 46, 38, 30, 22,
    14, 6,  61, 53, 45, 37, 29,
    21, 13, 5,  28, 20, 12, 4
};

// PC-2 (56bit -> 48bit)
static const uint8_t PC2[48] = {
    14, 17, 11, 24, 1,  5,
    3,  28, 15, 6,  21, 10,
    23, 19, 12, 4,  26, 8,
    16, 7,  27, 20, 13, 2,
    41, 52, 31, 37, 47, 55,
    30, 40, 51, 45, 33, 48,
    44, 49, 39, 56, 34, 53,
    46, 42, 50, 36, 29, 32
};

// 左移位数
static const uint8_t SHIFT[16] = {
    1, 1, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 1
};

// ==================== 辅助函数 ====================

// 获取指定位 (0-index, 从左到右)
static int get_bit(const uint8_t* data, int index) {
    return (data[index >> 3] >> (7 - (index & 7))) & 1;
}

// 设置指定位
static void set_bit(uint8_t* data, int index, int val) {
    if (val) {
        data[index >> 3] |= (1 << (7 - (index & 7)));
    }
    else {
        data[index >> 3] &= ~(1 << (7 - (index & 7)));
    }
}

// 置换操作
static void permutation(const uint8_t* input, uint8_t* output, const uint8_t* table, int len) {
    for (int i = 0; i < len; i++) {
        set_bit(output, i, get_bit(input, table[i] - 1));
    }
}

// 左移56位密钥
static void left_shift56(uint8_t* key56, int shift) {
    uint64_t k = 0;
    for (int i = 0; i < 56; i++) {
        if (get_bit(key56, i)) {
            k |= (1ULL << (55 - i));
        }
    }
    uint64_t mask = (1ULL << 28) - 1;
    uint64_t left = (k >> 28) & mask;
    uint64_t right = k & mask;
    left = ((left << shift) | (left >> (28 - shift))) & mask;
    right = ((right << shift) | (right >> (28 - shift))) & mask;
    k = (left << 28) | right;
    for (int i = 0; i < 56; i++) {
        set_bit(key56, i, (k >> (55 - i)) & 1);
    }
}

// 生成16轮子密钥
static void generate_subkeys(const uint8_t* key_8bytes, uint8_t subkeys[16][6]) {
    uint8_t key56[7] = { 0 };
    permutation(key_8bytes, key56, PC1, 56);

    for (int round = 0; round < 16; round++) {
        left_shift56(key56, SHIFT[round]);
        permutation(key56, subkeys[round], PC2, 48);
    }
}

// Feistel 函数
static void feistel(const uint8_t* right32, const uint8_t* subkey48, uint8_t* output32) {
    uint8_t expanded48[6] = { 0 };
    permutation(right32, expanded48, E, 48);

    // XOR with subkey
    for (int i = 0; i < 6; i++) {
        expanded48[i] ^= subkey48[i];
    }

    // S 盒替换 (48bit -> 32bit)
    uint8_t sbox_out[4] = { 0 };
    for (int i = 0; i < 8; i++) {
        int byte_idx = i * 6 / 8;
        int bit_offset = (i * 6) % 8;
        uint16_t six_bits = 0;
        for (int j = 0; j < 6; j++) {
            int bit_pos = bit_offset + j;
            if (bit_pos < 8) {
                if ((expanded48[byte_idx] >> (7 - bit_pos)) & 1) {
                    six_bits |= (1 << (5 - j));
                }
            }
            else {
                if ((expanded48[byte_idx + 1] >> (7 - (bit_pos - 8))) & 1) {
                    six_bits |= (1 << (5 - j));
                }
            }
        }
        int row = ((six_bits >> 4) & 2) | (six_bits & 1);
        int col = (six_bits >> 1) & 0xF;
        int s_val = S[i][row][col];
        int out_byte_idx = i * 4 / 8;
        int out_bit_offset = (i * 4) % 8;
        for (int j = 0; j < 4; j++) {
            int bit_pos = out_bit_offset + j;
            if (bit_pos < 8) {
                if (s_val & (1 << (3 - j))) {
                    sbox_out[out_byte_idx] |= (1 << (7 - bit_pos));
                }
            }
            else {
                if (s_val & (1 << (3 - j))) {
                    sbox_out[out_byte_idx + 1] |= (1 << (7 - (bit_pos - 8)));
                }
            }
        }
    }

    // P 置换
    permutation(sbox_out, output32, P, 32);
}

// DES 单块加密/解密 (8字节)
static void des_block(const uint8_t* input, uint8_t* output, const uint8_t subkeys[16][6], int decrypt) {
    uint8_t ip_output[8] = { 0 };
    permutation(input, ip_output, IP, 64);

    uint8_t left[4], right[4];
    memcpy(left, ip_output, 4);
    memcpy(right, ip_output + 4, 4);

    for (int round = 0; round < 16; round++) {
        uint8_t new_right[4] = { 0 };
        int key_idx = decrypt ? 15 - round : round;
        feistel(right, subkeys[key_idx], new_right);

        for (int i = 0; i < 4; i++) {
            new_right[i] ^= left[i];
        }
        memcpy(left, right, 4);
        memcpy(right, new_right, 4);
    }

    uint8_t combined[8] = { 0 };
    memcpy(combined, right, 4);
    memcpy(combined + 4, left, 4);

    permutation(combined, output, FP, 64);
}

// ==================== PKCS#7 填充 ====================

// 计算填充后的长度
static size_t get_padded_length(size_t original_len) {
    size_t pad_len = 8 - (original_len % 8);
    if (pad_len == 0) pad_len = 8;
    return original_len + pad_len;
}

// PKCS#7 填充
static void pkcs7_pad(const uint8_t* input, size_t input_len, uint8_t** output, size_t* output_len) {
    size_t pad_len = 8 - (input_len % 8);
    if (pad_len == 0) pad_len = 8;
    *output_len = input_len + pad_len;
    *output = (uint8_t*)malloc(*output_len);
    if (*output == NULL) return;

    memcpy(*output, input, input_len);
    for (size_t i = 0; i < pad_len; i++) {
        (*output)[input_len + i] = (uint8_t)pad_len;
    }
}

// PKCS#7 去除填充
static void pkcs7_unpad(uint8_t* input, size_t input_len, uint8_t** output, size_t* output_len) {
    if (input_len == 0) {
        *output = NULL;
        *output_len = 0;
        return;
    }
    uint8_t pad_len = input[input_len - 1];
    if (pad_len < 1 || pad_len > 8 || pad_len > input_len) {
        *output = NULL;
        *output_len = 0;
        return;
    }
    *output_len = input_len - pad_len;
    *output = (uint8_t*)malloc(*output_len);
    if (*output == NULL) return;
    memcpy(*output, input, *output_len);
}

// ==================== 公开 API ====================

// 加密函数
// 输入: plaintext - 明文数据, plaintext_len - 明文长度, key - 8字节密钥
// 输出: ciphertext_len - 密文长度
// 返回: 密文数据指针 (需要调用者 free)
uint8_t* des_encrypt(const uint8_t* plaintext, size_t plaintext_len, const uint8_t* key, size_t* ciphertext_len) {
    if (plaintext == NULL || key == NULL || ciphertext_len == NULL) {
        return NULL;
    }

    // 生成子密钥
    uint8_t subkeys[16][6];
    generate_subkeys(key, subkeys);

    // PKCS#7 填充
    uint8_t* padded = NULL;
    size_t padded_len = 0;
    pkcs7_pad(plaintext, plaintext_len, &padded, &padded_len);
    if (padded == NULL) {
        return NULL;
    }

    // 分配密文缓冲区
    *ciphertext_len = padded_len;
    uint8_t* ciphertext = (uint8_t*)malloc(*ciphertext_len);
    if (ciphertext == NULL) {
        free(padded);
        return NULL;
    }

    // 分块加密
    for (size_t i = 0; i < padded_len; i += 8) {
        des_block(padded + i, ciphertext + i, subkeys, 0);
    }

    free(padded);
    return ciphertext;
}

// 解密函数
// 输入: ciphertext - 密文数据, ciphertext_len - 密文长度, key - 8字节密钥
// 输出: plaintext_len - 明文长度
// 返回: 明文数据指针 (需要调用者 free)
uint8_t* des_decrypt(const uint8_t* ciphertext, size_t ciphertext_len, const uint8_t* key, size_t* plaintext_len) {
    if (ciphertext == NULL || key == NULL || plaintext_len == NULL) {
        return NULL;
    }

    // 密文长度必须是8的倍数
    if (ciphertext_len % 8 != 0) {
        return NULL;
    }

    // 生成子密钥
    uint8_t subkeys[16][6];
    generate_subkeys(key, subkeys);

    // 分配明文缓冲区
    uint8_t* padded = (uint8_t*)malloc(ciphertext_len);
    if (padded == NULL) {
        return NULL;
    }

    // 分块解密
    for (size_t i = 0; i < ciphertext_len; i += 8) {
        des_block(ciphertext + i, padded + i, subkeys, 1);
    }

    // 去除填充
    uint8_t* plaintext = NULL;
    pkcs7_unpad(padded, ciphertext_len, &plaintext, plaintext_len);

    free(padded);
    return plaintext;
}

// ==================== 测试示例 ====================

int des_test() {
    // 8字节固定密钥
    const char * key = "ocularv3";

    // 测试数据（任意长度）
    const char* test_str = "Hello DES! This is a secret message that can be of any length. 你好";
    size_t plaintext_len = strlen(test_str);
    uint8_t* plaintext = (uint8_t*)test_str;

    printf("原始明文: %s\n", plaintext);
    printf("明文长度: %zu 字节\n", plaintext_len);

    // 加密
    size_t ciphertext_len = 0;
    uint8_t* ciphertext = des_encrypt(plaintext, plaintext_len,(const unsigned char*) key, &ciphertext_len);
    if (ciphertext == NULL) {
        printf("加密失败！\n");
        return 1;
    }
    printf("加密完成，密文长度: %zu 字节\n", ciphertext_len);

    // 打印密文（十六进制）
    printf("密文(hex): ");
    for (size_t i = 0; i < ciphertext_len; i++) {
        printf("%02X", ciphertext[i]);
    }
    printf("\n");

    // 解密
    size_t decrypted_len = 0;
    uint8_t* decrypted = des_decrypt(ciphertext, ciphertext_len, (const unsigned char*)key, &decrypted_len);
    if (decrypted == NULL) {
        printf("解密失败！\n");
        free(ciphertext);
        return 1;
    }

    printf("解密后明文: %s\n", decrypted);
    printf("解密长度: %zu 字节\n", decrypted_len);
    printf("解密结果: %s\n",
        (plaintext_len == decrypted_len && memcmp(plaintext, decrypted, plaintext_len) == 0)
        ? "✓ 正确" : "✗ 错误");

    // 释放内存
    free(ciphertext);
    free(decrypted);

    return 0;
}