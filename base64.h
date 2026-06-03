#pragma once



#include <iostream>


int base24_decode(char* src, int srcsize, char* dst);

int base64_decode(const std::string& encoded_string,char * out);

std::string base64_encode(const char* bytes_to_encode, unsigned int in_len);
