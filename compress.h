#pragma once

#include <zlib.h>

int uncompress_deflate(char* inputf, char* outputf);

int zdecompress(Byte* zdata, uLong nzdata, Byte* data, uLong* ndata);

int dzFiles(unsigned char* input, int inSize, unsigned char* data, int dataSize, char* path);


class Compress {
public:
	static int CompressData(unsigned char * src, unsigned long srcsize, unsigned char * dst, unsigned long  *dstsize);

	static int UncompressData(unsigned char* src, unsigned long srcsize, unsigned char* dst, unsigned long* dstsize);


};