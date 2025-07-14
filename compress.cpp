
#include "..\\include\\zlib.h"
#include "..\\include\\zconf.h"

#include "compress.h"

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include "utils.h"


#ifdef _WIN64
#pragma comment(lib,"../lib/zlibstat.lib")
#else
#pragma comment(lib,"../lib/zlib.lib")
#endif

typedef DWORD(__stdcall* RtlCompressBuffer_Fn)(
	IN ULONG   CompressionFormat,
	IN PVOID   SourceBuffer,
	IN ULONG   SourceBufferLength,
	OUT PVOID   DestinationBuffer,
	IN ULONG   DestinationBufferLength,
	IN ULONG   Unknown,
	OUT PULONG   pDestinationSize,
	IN PVOID   WorkspaceBuffer);

typedef DWORD(__stdcall* RtlDecompressBuffer_Fn)(
	IN ULONG   CompressionFormat,
	OUT PVOID   DestinationBuffer,
	IN ULONG   DestinationBufferLength,
	IN PVOID   SourceBuffer,
	IN ULONG   SourceBufferLength,
	OUT PULONG   pDestinationSize);

typedef DWORD(__stdcall* RtlGetCompressionWorkSpaceSize_Fn)(
	IN ULONG   CompressionFormat,
	OUT PULONG   pNeededBufferSize,
	OUT PULONG   pUnknown);



int Compress::CompressData(unsigned char* src, unsigned long srcsize, unsigned char* dstbuf, unsigned long* dstsize) {
	int ret = 0;
	ret = compress(dstbuf, dstsize, src, srcsize);

	// 	HMODULE hntdll = LoadLibraryA("ntdll.dll");
	// 	if (hntdll != NULL)
	// 	{
	// 		RtlCompressBuffer_Fn fcmp = (RtlCompressBuffer_Fn)GetProcAddress(hntdll, "RtlCompressBuffer");
	// 		RtlDecompressBuffer_Fn fdcp = (RtlDecompressBuffer_Fn)GetProcAddress(hntdll, "RtlDecompressBuffer");
	// 		RtlGetCompressionWorkSpaceSize_Fn fgcw = (RtlGetCompressionWorkSpaceSize_Fn)GetProcAddress(hntdll, "RtlGetCompressionWorkSpaceSize");
	// 
	// 
	// 		if (fcmp && fdcp && fgcw)
	// 		{
	// 			DWORD resultSize = 0;
	// 			unsigned long x = 0;
	// 			ret = (*fgcw)(COMPRESSION_FORMAT_LZNT1 | COMPRESSION_ENGINE_MAXIMUM, &resultSize, &x);
	// 			void * tmp = LocalAlloc(LPTR, resultSize);
	// 
	// 			ret = (*fcmp)(COMPRESSION_FORMAT_LZNT1 | COMPRESSION_ENGINE_MAXIMUM, src, srcsize, dstbuf, *dstsize, x, &resultSize, tmp);
	// 
	// 			LocalFree(tmp);
	// 
	// 			*dstsize = resultSize;
	// 
	// 		}
	// 	}

	return ret;
}



int Compress::UncompressData(unsigned char* src, unsigned long srcsize, unsigned char* dstbuf, unsigned long* dstsize)
{
	int ret = 0;
	ret = uncompress(dstbuf, dstsize, src, srcsize);
	return ret;
}




#include <stdio.h>
#include <string.h>
#include <zlib.h>

#define CHUNK_SIZE 16384

int uncompress_deflate(char* inputf, char* outputf) {

	FILE* input = fopen(inputf, "rb");
	FILE* output = fopen(outputf, "wb");

	z_stream stream;
	unsigned char in_buf[CHUNK_SIZE];
	unsigned char out_buf[CHUNK_SIZE];

	memset(&stream, 0, sizeof(stream));

	// 初始化 zlib，使用 -MAX_WBITS 处理原始 Deflate
	int ret = inflateInit2(&stream, -MAX_WBITS);
	if (ret != Z_OK) {
		fprintf(stderr, "inflateInit failed: %s\n", stream.msg);
		return -1;
	}

	do {
		stream.avail_in = fread(in_buf, 1, CHUNK_SIZE, input);
		if (ferror(input)) {
			inflateEnd(&stream);
			return -1;
		}
		if (stream.avail_in == 0) break;

		stream.next_in = in_buf;
		stream.next_out = out_buf;
		stream.avail_out = CHUNK_SIZE;

		ret = inflate(&stream, Z_NO_FLUSH);
		if (ret == Z_STREAM_ERROR) {
			fprintf(stderr, "inflate error: %s\n", stream.msg);
			inflateEnd(&stream);
			return -1;
		}

		size_t decompressed_size = CHUNK_SIZE - stream.avail_out;
		fwrite(out_buf, 1, decompressed_size, output);
	} while (ret != Z_STREAM_END);

	inflateEnd(&stream);
	return (ret == Z_STREAM_END) ? 0 : -1;
}


int zdecompress(Byte* zdata, uLong nzdata, Byte* data, uLong* ndata)
{
	int err = 0;
	z_stream d_stream; /* decompression stream */

	d_stream.zalloc = (alloc_func)0;
	d_stream.zfree = (free_func)0;
	d_stream.opaque = (voidpf)0;
	d_stream.next_in = zdata;
	d_stream.avail_in = 0;
	d_stream.next_out = data;
	if (inflateInit(&d_stream) != Z_OK)
		return -1;
	while (d_stream.total_out < *ndata && d_stream.total_in < nzdata) {
		d_stream.avail_in = d_stream.avail_out = 1; /* force small buffers */
		if ((err = inflate(&d_stream, Z_NO_FLUSH)) == Z_STREAM_END)
			break;
		if (err != Z_OK)
			return -1;
	}
	if (inflateEnd(&d_stream) != Z_OK)
		return -1;
	*ndata = d_stream.total_out;
	return d_stream.next_in - zdata;
}



int dzFiles(unsigned char*input,int inSize, unsigned char*data,int dataSize, char * path) {
	int ret = 0;
	char ofn[1024];

	int error = 0;

	__int64 num = (__int64)0;

	char newpath[1024] = { 0 };
	newpath[0] = '.';
	newpath[1] = '\\';
	lstrcatA(newpath, path);
	ret = CreateDirectoryA(newpath, 0);

	int zTotal = 0;
	unsigned char* zdata = (unsigned char*)input;
	unsigned long zSize = inSize;
	unsigned char* dzData = (unsigned char*)data;
	unsigned long dzSize = dataSize;
	while (zTotal < inSize) {
		if (memcmp(zdata, "\x78\x9c", 2) != 0) {
			for (int i = 0; i < 0x1000; i++) {
				if (zdata + i >= input + inSize) {
					error = 1;
					break;
				}
				if (memcmp(zdata + i, "\x78\x9c", 2) == 0) {
					zdata += i;
					zSize -= i;
					break;
				}
			}
		}
		if (error) {
			break;
		}
		int zlen = zdecompress((unsigned char*)zdata, zSize, (unsigned char*)dzData, (unsigned long*)&dzSize);
		if (zlen > 0) {
			zdata += zlen;
			zdata += 10;
			zSize -= zlen;
			zSize -= 10;

			zTotal += dzSize;

			wsprintfA(ofn, ".\\%s\\%08I64d", path, num);
			num++;
			FWriter(ofn, (char*)dzData, dzSize, 0);

			dzData += zTotal;
			dzSize = dataSize - zTotal;
		}
		else {
			printf("error:%d\r\n", zlen);
			break;
		}
	}
	return num;
}