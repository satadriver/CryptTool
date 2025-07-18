#pragma once

#include <zlib.h>

#include <stdio.h>
#include <string.h>

#include <string>

#define CHUNK_SIZE 16384


using namespace std;

int uncompress_deflate(char* inputf, char* outputf);

int zdecompress(Byte* zdata, uLong nzdata, Byte* data, uLong* ndata);

int dzFiles(unsigned char* input, int inSize, unsigned char* data, int dataSize, char* path);


class Compress {
public:
	static int CompressData(unsigned char * src, unsigned long srcsize, unsigned char * dst, unsigned long  *dstsize);

	static int UncompressData(unsigned char* src, unsigned long srcsize, unsigned char* dst, unsigned long* dstsize);

	int gzcompress(Bytef* data, uLong ndata, Bytef* zdata, uLong* nzdata);

	int httpgzdecompress(Byte* zdata, uLong nzdata, Byte* data, uLong* ndata);

	int gzfile(string srcfn, string dstfn, int withname, string ingzfn);

	int gzfiledata(Byte* data, uLong ndata, Byte* gzdata, uLong* ngzdata);

	int zcompress(Bytef* data, uLong ndata, Bytef* zdata, uLong* nzdata);

	int zdecompress(Byte* zdata, uLong nzdata, Byte* data, uLong* ndata);

	int gzdecompress(Byte* zdata, uLong nzdata, Byte* data, uLong* ndata);

	static void testcompress();
};