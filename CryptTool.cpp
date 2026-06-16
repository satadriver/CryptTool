

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
#include "url.h"

using namespace std;


//--search function "root_user" "D:\work\ftos\_FTOS-SK-9.14.1.7.bin.extracted\1102787"
//--search b "\x00\x01\x02\x03\x04\x05" "C:\Users\liujinguang01\Desktop\intouch"

int main(int argc,char ** argv)
{
	int ret = 0;

	if (argc <= 1) {
		printf("example:%s [command] [in] [out]\r\n", argv[0]);
		return FALSE;
	}

	int action = 0;

	char* infn = 0;
	char* outfn = 0;

	char* output = 0;
	char * input =  0;

	unsigned __int64 inSize = 0;
	__int64 outSize = 0;

	char* option = 0;

	for (int seq = 1; seq < argc; ) {
		if (lstrcmpiA(argv[seq], "-enc") == 0) {
			action = BASE64_ENCODE;
			seq++;
		}
		else if (lstrcmpiA(argv[seq], "-dec") == 0) {
			action = BASE64_DECODE;
			seq++;
		}
		else if (lstrcmpiA(argv[seq], "-b24") == 0) {
			action = BASE24_CODE;
			seq++;
		}
		else if (lstrcmpiA(argv[seq], "-urldec") == 0) {
			action = URL_DECODE;
			seq++;
		}
		else if (lstrcmpiA(argv[seq], "-urlenc") == 0) {
			action = URL_ENCODE;
			seq++;
		}
		else if (lstrcmpiA(argv[seq], "-sha1") == 0) {
			action = SHA1_ENCODE;
			seq++;
		}
		else if (lstrcmpiA(argv[seq], "-md5") == 0) {
			action = MD5_ENCODE;
			seq++;
		}
		else if (lstrcmpiA(argv[seq], "-zdecomp") == 0) {
			action = ZDECOMPRESS;
			seq++;			
		}
		else if (lstrcmpiA(argv[seq], "-zcomp") == 0) {
			action = ZCOMPRESS;
			seq++;
		}
		else if (lstrcmpiA(argv[seq], "-gzdecomp") == 0) {
			action = GZDECOMPRESS;
			seq++;
		}
		else if (lstrcmpiA(argv[seq], "-gzcompress") == 0) {
			action = GZCOMPRESS;
			seq++;
		}
		else if (lstrcmpiA(argv[seq], "-network") == 0) {
			action = NETWORKTEST;
			option = argv[seq + 1];
			input = argv[seq + 2];
			infn = argv[seq + 3];
			inSize = (unsigned __int64)argv[seq + 4];
			seq += 5;
		}
		else if (lstrcmpiA(argv[seq], "--if") == 0) {
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
		else if (lstrcmpiA(argv[seq], "--is") == 0) {
			input = argv[seq + 1];
			inSize = lstrlenA(input);
			seq += 2;
		}
		else if (lstrcmpiA(argv[seq], "--of") == 0) {

			outfn = argv[seq + 1];
			seq += 2;
		}
		else if (lstrcmpiA(argv[seq], "--os") == 0) {

			outfn = argv[seq + 1];
			seq += 1;
		}
		else if (lstrcmpiA(argv[seq], "-splitfile") == 0) {
			action = SPLIT_FILE_WITH_TAG;
			seq++;
		}
		else if (lstrcmpiA(argv[seq], "-search") == 0) {
			action = STRINGSEARCH;
			option = argv[seq + 1];
			input = argv[seq + 2];
			infn = argv[seq + 3];
			inSize = lstrlenA(input);
			seq+=4;
		}
		else if (lstrcmpA(argv[seq], "-proxy") == 0) {
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
		output = (char*) outstr.c_str();
		outSize = outstr.length();
	}
	else if (action == BASE64_DECODE) {
		string instr = input;
		char* out = new char[instr.length() + 16];
		int outlen = base64_decode(instr,out);
		output = out;
		outSize = outlen;
	}
	else if (action == BASE24_CODE) {
		output = new char[inSize + 16];
		outSize = base24_decode(input,(int) inSize,output);
	}
	else if (action == URL_DECODE) {
		output = new char[inSize + 1024];
		outSize = Url::urldecode(input, (int)inSize, output);
	}
	else if (action == URL_ENCODE) {
		output = new char[inSize*2 + 1024];
		outSize = Url::urlencode(input, (int)inSize, output,inSize + 16);
	}
	else if (action == SHA1_ENCODE) {
		if (output == 0) {
			output = new char[1024];
		}
		ret = sha1((unsigned char*)input, inSize, (unsigned char*)output);
		outSize = 20;
	}
	else if (action == MD5_ENCODE) {
		if (output == 0) {
			output = new char[1024];
		}
		ret = GetMD5((unsigned char*)input, inSize, (unsigned char*)output, 0);
		outSize = 16;
	}
	else if (action == ZCOMPRESS) {
		if (output == 0) {
			outSize = inSize + 0x1000;
			output = new char[outSize];
		}
		
		ret = Compress::zcompress((unsigned char*)input, inSize, (unsigned char*)output,(unsigned long*) &outSize);
	}
	else if (action == ZDECOMPRESS) {
		if (output == 0) {
			outSize = inSize*16 + 0x1000;
			output = new char[outSize];
		}
		ret = Compress::zdecompress((unsigned char*)input, inSize, (unsigned char*)output, (unsigned long*)&outSize);
	}
	else if (action == ZDECOMPRESSF) {
		if (output == 0) {
			outSize = inSize * 16 + 0x1000;
			output = new char[outSize];
		}
		//ret = Compress::UncompressData((unsigned char*)input, inSize, (unsigned char*)data, (unsigned long*)&dataSize);
		//uncompress_deflate(infn, outfn);

		char filepath[1024];
		ret = GetNameFromPath(infn, filepath);
		int num = dzFiles((unsigned char*)input, inSize, (unsigned char*)output, outSize, filepath);
	}
	else if (action == GZCOMPRESS) {
		if (output == 0) {
			outSize = inSize + 0x1000;
			output = new char[outSize];
		}
		ret = Compress::gzcompress((unsigned char*)input, inSize, (unsigned char*)output, (unsigned long*)&outSize);
	}
	else if (action == GZDECOMPRESS) {
		if (output == 0) {
			outSize = inSize*16 + 0x1000;
			output = new char[inSize + 0x1000];
		}
		ret = Compress::gzdecompress((unsigned char*)input, inSize, (unsigned char*)output, (unsigned long*)&outSize);
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
		ret = Network(argc-2,&argv[2]);
	}
	else {
		
	}

	if (outfn) {
		ret = FWriter(outfn, output, outSize, 0);
	}
	return 0;
}


