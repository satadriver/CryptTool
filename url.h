#pragma once


#include <iostream>

using namespace std;

class Url {
public:
	static int urlencode(char* in_str, int insize, char* out_str, int outsize);
	static int urldecode(char* src, int srclen,char * dst);
};