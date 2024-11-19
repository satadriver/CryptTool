
#include <iostream>
#include <windows.h>
#include "utils.h"
#include "base64.h"
#include "md5.h"
#include "sha1.h"

using namespace std;


#define DEFAULT_OUTPUT_FN	"out.txt"



int main(int argc,char ** argv)
{
	int ret = 0;

	if (argc <= 1) {
		return FALSE;
	}

	for (int i = 1; i < argc; i++) {
		if (lstrcmpiA(argv[i], "--encode") == 0) {
			string code = "";
			char * in = argv[i + 2];
			if (lstrcmpiA(argv[i + 1], "-if") == 0) {
				char* data = 0;
				int fs = 0;
				ret = FReader(in, &data, &fs);
				if (ret) {
					code = base64_encode(data, fs);
				}			
			}
			else if (lstrcmpiA(argv[i + 1], "-is") == 0) {
				code = base64_encode(in, lstrlenA(in));
			}
			else if (lstrcmpiA(argv[i + 1], "-i") == 0) {
				unsigned char base64[0x1000];
				int len = str2hex((unsigned char*)in, base64);
				code = base64_encode((char*)base64, len);
			}
			else {
				printf("%s base64 param error\r\n", __FUNCTION__);
				return FALSE;
			}

			if (lstrcmpiA(argv[i + 3], "-of") == 0) {
				char* out = argv[i + 4];
				ret = FWriter(out, code.c_str(), code.length(),0);
				printf("encode result:%s\r\n", code.c_str());
			}
			else if (lstrcmpiA(argv[i + 3], "-o") == 0) {
				const char* out = DEFAULT_OUTPUT_FN;
				ret = FWriter(out, code.c_str(), code.length(), 0);
				printf("encode result:%s\r\n", code.c_str());
			}
			else {
				printf("encode result:%s\r\n", code.c_str());
			}
			break;
		}
		else if (lstrcmpiA(argv[i], "--decode") == 0) {
			string code = "";
			char* in = argv[i + 2];
			if (lstrcmpiA(argv[i + 1], "-if") == 0) {
				char* data = 0;
				int fs = 0;
				int ret = FReader(in, &data, &fs);
				string base64 = string(data,fs);
				code = base64_decode(code);
			}
			else if (lstrcmpiA(argv[i + 1], "-is") == 0) {
				code = base64_decode(in);
			}
			else if (lstrcmpiA(argv[i + 1], "-o") == 0) {
				unsigned char base64[0x1000];
				int len = str2hex((unsigned char*)in, base64);
				code = base64_decode((char*)base64);
			}
			else {
				printf("%s base64 param error\r\n", __FUNCTION__);
				return FALSE;
			}

			if (lstrcmpiA(argv[i + 3], "-of") == 0) {
				char* out = argv[i + 4];
				int ret = FWriter(out, code.c_str(), code.length(), 0);
				printf("decode result:%s\r\n", code.c_str());
			}
			else if (lstrcmpiA(argv[i + 3], "-o") == 0) {
				const char* out = DEFAULT_OUTPUT_FN;
				ret = FWriter(out, code.c_str(), code.length(), 0);
				printf("decode result:%s\r\n", code.c_str());
			}
			else {
				printf("decode result:%s\r\n", code.c_str());
			}
			break;
		}
		else if (lstrcmpiA(argv[i], "--sha1") == 0) {
			unsigned char code[32];
			int codeLen = 20;
			char* in = argv[i + 2];
			if (lstrcmpiA(argv[i + 1], "-if") == 0) {
				char* data = 0;
				int fs = 0;
				ret = FReader(in, &data, &fs);
				if (ret) {
					sha1((unsigned char*)data, fs, code);
				}		
			}
			else if (lstrcmpiA(argv[i + 1], "-is") == 0) {
				sha1((unsigned char*)in, lstrlenA(in), code);
			}
			else if (lstrcmpiA(argv[i + 1], "-i") == 0) {
				unsigned char base64[0x1000];
				int len = str2hex((unsigned char*)in, base64);
				sha1((unsigned char*)base64, len, code);
			}
			else {
				printf("%s sha1 param error\r\n", __FUNCTION__);
				return FALSE;
			}

			if (lstrcmpiA(argv[i + 3], "-of") == 0) {
				char* out = argv[i + 4];
				ret = FWriter(out, (char*)code, codeLen, 0);
			}
			else if (lstrcmpiA(argv[i + 3], "-o") == 0) {
				const char* out = DEFAULT_OUTPUT_FN;
				ret = FWriter(out, (char*)code, codeLen, 0);
			}
			else {

			}
			code[codeLen] = 0;
			printf("sha1 result:\r\n");
			hex2str((char*)code, codeLen);
			break;
		}
		else if (lstrcmpiA(argv[i], "--md5") == 0) {
			unsigned char code[20];
			int codeLen = 16;
			char* in = argv[i + 2];
			if (lstrcmpiA(argv[i + 1], "-if") == 0) {
				char* data = 0;
				int fs = 0;
				ret = FReader(in, &data, &fs);
				if (ret) {
					GetMD5((unsigned char*)data, fs, code, 0);
				}
				

			}
			else if (lstrcmpiA(argv[i + 1], "-is") == 0) {
				GetMD5((unsigned char*)in, lstrlenA(in), code, 0);
			}
			else if (lstrcmpiA(argv[i + 1], "-i") == 0) {
				unsigned char base64[0x1000];
				int len = str2hex((unsigned char*)in, base64);
				GetMD5((unsigned char*)in, len, code, 0);
			}
			else {
				printf("%s md5 param error\r\n", __FUNCTION__);
				return FALSE;
			}

			if (lstrcmpiA(argv[i + 3], "-of") == 0) {
				char* out = argv[i + 4];
				ret = FWriter(out, (char*)code, codeLen, 0);
			}
			else if (lstrcmpiA(argv[i + 3], "-o") == 0) {
				const char* out = DEFAULT_OUTPUT_FN;
				ret = FWriter(out, (char*)code, codeLen, 0);
			}
			else {

			}
			printf("md5 result:\r\n");
			hex2str((char*)code, codeLen);
			break;
		}
		else {
			break;
		}
	}

	return 0;
}


