

// 正确顺序
#define _WIN32_WINNT 0x0A00
#include <windows.h>


#include <iostream>

#include "utils.h"
#include "base64.h"
#include "md5.h"
#include "sha1.h"
#include "compress.h"
#include "split2file.h"
#include "grepFunc.h"
#include "FileUtils.h"
#include "Proxy.h"
#include "CryptTool.h"
#include "network.h"


using namespace std;



int main(int argc,char ** argv)
{
	int ret = 0;

	//Compress::testcompress();

	if (argc <= 1) {
		return FALSE;
	}

	int action = 0;

	char* infn = 0;

	char * input =  0;
	unsigned __int64 inSize = 0;

	char* outfn = 0;

	char* data = 0;
	__int64 dataSize = 0;

	char* option = 0;

	for (int seq = 1; seq < argc; ) {
		if (lstrcmpiA(argv[seq], "--encode") == 0) {
			action = BASE64_ENCODE;
			seq++;
		}
		else if (lstrcmpiA(argv[seq], "--decode") == 0) {
			action = BASE64_DECODE;
			seq++;
		}
		else if (lstrcmpiA(argv[seq], "--sha1") == 0) {
			action = SHA1_ENCODE;
			seq++;
		}
		else if (lstrcmpiA(argv[seq], "--md5") == 0) {
			action = MD5_ENCODE;
			seq++;
		}
		else if (lstrcmpiA(argv[seq], "--uncompress") == 0) {
			action = UNCOMPRESS;
			seq++;			
		}
		else if (lstrcmpiA(argv[seq], "--compress") == 0) {
			action = COMPRESS;
			seq++;
		}
		else if (lstrcmpiA(argv[seq], "--networktest") == 0) {
			action = NETWORKTEST;
			option = argv[seq + 1];
			input = argv[seq + 2];
			infn = argv[seq + 3];
			inSize = (unsigned __int64)argv[seq + 4];
			seq += 5;
		}
		else if (lstrcmpiA(argv[seq], "-if") == 0) {
			infn = argv[seq + 1];
			if (infn == 0) {
				break;
			}

			ret = FReader(infn, &input, &inSize);
			if (ret == 0) {
				break;
			}

			seq += 2;
		}
		else if (lstrcmpiA(argv[seq], "-is") == 0) {
			input = argv[seq + 1];
			inSize = lstrlenA(input);
			seq += 2;
		}
		else if (lstrcmpiA(argv[seq], "-of") == 0) {

			outfn = argv[seq + 1];
			seq += 2;
		}
		else if (lstrcmpiA(argv[seq], "--splitfile") == 0) {
			action = SPLIT_FILE_WITH_TAG;
			seq++;
		}
		else if (lstrcmpiA(argv[seq], "--search") == 0) {
			action = STRINGSEARCH;
			option = argv[seq + 1];
			input = argv[seq + 2];
			infn = argv[seq + 3];
			inSize = lstrlenA(input);
			seq+=4;
		}
		else if (lstrcmpA(argv[seq], "--proxy") == 0) {
			action = NETWORKPROXY;
			seq++;
		}
		else {
			seq++;
		}
	}

	if ( input == 0 || action == 0 || inSize == 0) {
		return -1;
	}

	if (action == BASE64_ENCODE) {
		string outstr = (char*)base64_encode(input, inSize).c_str();
		data = (char*) outstr.c_str();
		dataSize = outstr.length();
	}
	else if (action == BASE64_DECODE) {
		string instr = input;
		string outstr = base64_decode(instr);
		data = (char*)outstr.c_str();
		dataSize = outstr.length();
	}
	else if (action == SHA1_ENCODE) {
		if (data == 0) {
			data = new char[1024];
		}
		ret = sha1((unsigned char*)input, inSize, (unsigned char*)data);
		dataSize = 20;
	}
	else if (action == MD5_ENCODE) {
		if (data == 0) {
			data = new char[1024];
		}
		ret = GetMD5((unsigned char*)input, inSize, (unsigned char*)data, 0);
		dataSize = 16;
	}
	else if (action == COMPRESS) {
		if (data == 0) {
			data = new char[inSize+0x1000];
		}
		ret = Compress::CompressData((unsigned char*)input, inSize, (unsigned char*)data,(unsigned long*) &dataSize);
	}
	else if (action == UNCOMPRESS) {
		if (data == 0) {
			dataSize = inSize * 16 + 0x1000;
			data = new char[dataSize];
		}
		//ret = Compress::UncompressData((unsigned char*)input, inSize, (unsigned char*)data, (unsigned long*)&dataSize);
		//uncompress_deflate(infn, outfn);

		char filepath[1024];
		ret = GetNameFromPath(infn, filepath);
		int num = dzFiles((unsigned char*)input, inSize, (unsigned char*)data, dataSize, filepath);
	}
	else if (action == SPLIT_FILE_WITH_TAG) {
		char filepath[1024];
		ret = GetNameFromPath(infn, filepath);
		ret = Split2File(input, inSize, filepath, (unsigned char*)"\x7f\x45\x4c\x46",4);
	}
	else if (action == STRINGSEARCH) {
		ret = SearchString(option,input,infn );
	}
	else if (action == NETWORKPROXY) {
		//ret = NetworkProxy();
	}
	else if (action == NETWORKTEST) {
		ret = TestNetwork(option,input, infn,(char*) inSize);
	}
	else {
		
	}

	if (outfn) {
		ret = FWriter(outfn, data, dataSize, 0);
	}
	return 0;
}


