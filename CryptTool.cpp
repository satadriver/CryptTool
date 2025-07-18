
#include <iostream>
#include <windows.h>
#include "utils.h"
#include "base64.h"
#include "md5.h"
#include "sha1.h"
#include "compress.h"
#include "binwalkELF.h"


using namespace std;


#define DEFAULT_OUTPUT_FN	"out.txt"

#define BASE64_ENCODE	1
#define BASE64_DECODE	2
#define SHA1_ENCODE		3
#define MD5_ENCODE		5
#define COMPRESS		7
#define UNCOMPRESS		8

#define SPLIT_BINWALK_ELF		0xffffffff

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
	__int64 inSize = 0;

	char* outfn = 0;

	char* data = 0;
	__int64 dataSize = 0;

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
		else if (lstrcmpiA(argv[seq], "-of") == 0) {

			outfn = argv[seq + 1];
			seq += 2;
		}
		else if (lstrcmpiA(argv[seq], "--split_binwalk_elf") == 0) {
			action = SPLIT_BINWALK_ELF;
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
	else if (action == SPLIT_BINWALK_ELF) {
		char filepath[1024];
		ret = GetNameFromPath(infn, filepath);
		ret = SplitBinwalkELF(input, inSize, filepath);
	}
	else {

	}

	if (outfn) {
		ret = FWriter(outfn, data, dataSize, 0);
	}
	return 0;
}


