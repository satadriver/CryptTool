
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






int uncompress_deflate(char* inputf, char* outputf) {

	FILE* input = fopen(inputf, "rb");
	FILE* output = fopen(outputf, "wb");

	z_stream stream;
	unsigned char in_buf[CHUNK_SIZE];
	unsigned char out_buf[CHUNK_SIZE];

	memset(&stream, 0, sizeof(stream));

	// ��ʼ�� zlib��ʹ�� -MAX_WBITS ����ԭʼ Deflate
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








/* Compress data */
int Compress::zcompress(Bytef* data, uLong ndata, Bytef* zdata, uLong* nzdata)
{
	z_stream c_stream;
	int err = 0;

	if (data && ndata > 0)
	{
		c_stream.zalloc = (alloc_func)0;
		c_stream.zfree = (free_func)0;
		c_stream.opaque = (voidpf)0;
		if (deflateInit(&c_stream, Z_DEFAULT_COMPRESSION) != Z_OK)
			return -1;
		c_stream.next_in = data;
		c_stream.avail_in = ndata;
		c_stream.next_out = zdata;
		c_stream.avail_out = *nzdata;
		while (c_stream.avail_in != 0 && c_stream.total_out < *nzdata)
		{
			if (deflate(&c_stream, Z_NO_FLUSH) != Z_OK)
				return -1;
		}
		if (c_stream.avail_in != 0)
			return c_stream.avail_in;
		for (;;) {
			if ((err = deflate(&c_stream, Z_FINISH)) == Z_STREAM_END)
				break;
			if (err != Z_OK)
				return -1;
		}
		if (deflateEnd(&c_stream) != Z_OK)
			return -1;
		*nzdata = c_stream.total_out;
		return 0;
	}
	return -1;
}

/* Uncompress data */
int Compress::zdecompress(Byte* zdata, uLong nzdata, Byte* data, uLong* ndata)
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
	return 0;
}




//GZIP��ʽ��־ 1f8b

//CM ѹ������(0-7 reserved, 8 = deflate)

//FLG��1 byte������־λ��    
//bit 0 set: FTEXT �ļ�������ASCII�ı��ļ� 
//bit 1 set: FHCRC ���Ӷ��gzip�ļ����� 
//bit 2 set: FEXTRA �����п�ѡ�ĸ��� ���� 
//bit 3 set: FNAME �ṩ��ԭʼ���ļ����� 
//bit 4 set: FCOMMENT ���ṩ��һ��O���ս���ļ����� 
//bit 5 set: �ļ������� 
//bit 6,7:   ���� 

//MTIME��4 byte�����ļ�����ʱ��(Unixʱ��)

//XFL��1 byte�������ӵı�־��������ѹ����������CM = 8ʱ��XFL = 2 �C ���ѹ�����������㷨��XFL = 4 �C ��쵫��Сѹ�����㷨

//OS��1 byte���������־ָ���˽���ѹ��ʱϵͳ�����͡�
//0 �C FAT filesystem (MS-DOS, OS/2, NT/Win32) 
//1 �C Amiga 
//2 �C VMS (or OpenVMS) 
//3 �C Unix 
//4 �C VM/CMS 
//5 �C Atari TOS 
//6 �C HPFS filesystem (OS/2, NT) 
//7 �C Macintosh 
//8 �C Z-System 
//9 �C CP/M 
//10 �C TOPS-20 
//11 �C NTFS filesystem (NT) 
//12 �C QDOS 
//13 �C Acorn RISCOS 
//255 �C unknown 

//ͷ����չ�ֶ�,����˳�������ǣ�FEXTRA+FNAME+FCOMMENT+FHCRC����һ��������ڣ�����ֻҪ���ڣ����۴��ڼ�����һ��Ҫ����˳����

//DATA

//CRC32��4 byte���������δѹ�����ݵ�ѭ������У��ֵ��
//ISIZE��4 byte��������ԭʼ���ݵĳ�����2��32�η�Ϊģ��ֵ��GZIP���ֽ�����˳����LSB��ʽ����Little - Endian����ZLIB�е��෴��

int Compress::gzfiledata(Byte* data, uLong ndata, Byte* gzdata, uLong* ngzdata) {
	DWORD crc = crc32(0, data, ndata);

	int ret = 0;

	memcpy(gzdata, "\x1f\x8b\x08\x00\x00\x00\x00\x00\x00\x03", 10);

	DWORD gzsize = *ngzdata - 10 - 4 - 4;

	ret = Compress::gzcompress(data, ndata, gzdata + 10, &gzsize);
	memcpy(gzdata + 10 + gzsize, &crc, 4);
	memcpy(gzdata + 10 + gzsize + 4, &ndata, 4);

	*ngzdata = gzsize + 18;

	return gzsize + 18;
}


/*
int Compress::gzfile(string srcfn, string dstfn, int withname, string ingzfn) {
	int ret = 0;

	//string filename = Public::getNameFromFullPath(dstfn);

	unsigned char* data = 0;
	DWORD ndata = 0;

	ret = FileOper::fileDecryptReader(srcfn, (char**)&data, (int*)&ndata);
	if (ret <= 0)
	{
		return FALSE;
	}

	DWORD crc = crc32(0, data, ndata);

	int ngzdata = ndata + 0x1000;
	unsigned char* gzdata = new unsigned char[ngzdata];

	int gzhdrsize = 10;
	if (withname)
	{
		memcpy(gzdata, "\x1f\x8b\x08\x08\x00\x00\x00\x00\x00\x00", gzhdrsize);

		memcpy(gzdata + gzhdrsize, ingzfn.c_str(), ingzfn.length() + 1);

		gzhdrsize += (ingzfn.length() + 1);
	}
	else {
		memcpy(gzdata, "\x1f\x8b\x08\x00\x00\x00\x00\x00\x00\x00", gzhdrsize);
	}

	DWORD gzsize = ngzdata - gzhdrsize - 8;

	ret = Compress::gzcompress(data, ndata, gzdata + gzhdrsize, &gzsize);
	if (ret < 0)
	{
		delete gzdata;
		return FALSE;
	}

	memcpy(gzdata + gzhdrsize + gzsize, &crc, 4);

	memcpy(gzdata + gzhdrsize + gzsize + 4, &ndata, 4);

	ret = FileOper::fileWriter(dstfn, (char*)gzdata, gzhdrsize + gzsize + 8, 1);

	delete gzdata;

	return gzsize + gzhdrsize + 8;
}
*/


/* Compress gzip data */
int Compress::gzcompress(Bytef* data, uLong ndata, Bytef* zdata, uLong* nzdata)
{
	z_stream c_stream;
	int err = 0;

	if (data && ndata > 0)
	{
		c_stream.zalloc = (alloc_func)0;
		c_stream.zfree = (free_func)0;
		c_stream.opaque = (voidpf)0;
		if (deflateInit2(&c_stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, -MAX_WBITS, 8, Z_DEFAULT_STRATEGY) != Z_OK)

			//ֻ������ΪMAX_WBITS + 16��������ѹ���ı��д�header��trailer

			//if (deflateInit2(&c_stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED,MAX_WBITS + 16, 8, Z_DEFAULT_STRATEGY) != Z_OK)
		{
			return -1;
		}

		c_stream.next_in = data;
		c_stream.avail_in = ndata;
		c_stream.next_out = zdata;
		c_stream.avail_out = *nzdata;
		while (c_stream.avail_in != 0 && c_stream.total_out < *nzdata)
		{
			if (deflate(&c_stream, Z_NO_FLUSH) != Z_OK)
			{
				return -1;
			}
		}

		if (c_stream.avail_in != 0)
		{
			return c_stream.avail_in;
		}

		for (;;) {
			if ((err = deflate(&c_stream, Z_FINISH)) == Z_STREAM_END)
			{
				break;
			}

			if (err != Z_OK)
			{
				return -1;
			}
		}

		if (deflateEnd(&c_stream) != Z_OK)
		{
			return -1;
		}

		*nzdata = c_stream.total_out;

		return 0;
	}
	return -1;
}



/* HTTP gzip decompress */
int Compress::httpgzdecompress(Byte* zdata, uLong nzdata, Byte* data, uLong* ndata)
{
	int err = 0;
	z_stream d_stream = { 0 }; /* decompression stream */
	static char dummy_head[2] =
	{
		0x8 + 0x7 * 0x10,
		(((0x8 + 0x7 * 0x10) * 0x100 + 30) / 31 * 31) & 0xFF,
	};
	d_stream.zalloc = (alloc_func)0;
	d_stream.zfree = (free_func)0;
	d_stream.opaque = (voidpf)0;
	d_stream.next_in = zdata;
	d_stream.avail_in = 0;
	d_stream.next_out = data;
	if (inflateInit2(&d_stream, -MAX_WBITS) != Z_OK)
		return -1;
	while (d_stream.total_out < *ndata && d_stream.total_in < nzdata) {
		d_stream.avail_in = d_stream.avail_out = 1; /* force small buffers */
		if ((err = inflate(&d_stream, Z_NO_FLUSH)) == Z_STREAM_END)
			break;
		if (err != Z_OK)
		{
			if (err == Z_DATA_ERROR)
			{
				d_stream.next_in = (Bytef*)dummy_head;
				d_stream.avail_in = sizeof(dummy_head);
				if ((err = inflate(&d_stream, Z_NO_FLUSH)) != Z_OK)
				{
					return -1;
				}
			}
			else return -1;
		}
	}
	if (inflateEnd(&d_stream) != Z_OK)
		return -1;
	*ndata = d_stream.total_out;
	return 0;
}

/* Uncompress gzip data */
int Compress::gzdecompress(Byte* zdata, uLong nzdata, Byte* data, uLong* ndata)
{
	int err = 0;
	z_stream d_stream = { 0 }; /* decompression stream */
	static char dummy_head[2] =
	{
		0x8 + 0x7 * 0x10,
		(((0x8 + 0x7 * 0x10) * 0x100 + 30) / 31 * 31) & 0xFF,
	};
	d_stream.zalloc = (alloc_func)0;
	d_stream.zfree = (free_func)0;
	d_stream.opaque = (voidpf)0;
	d_stream.next_in = zdata;
	d_stream.avail_in = 0;
	d_stream.next_out = data;
	if (inflateInit2(&d_stream, -MAX_WBITS) != Z_OK)
		return -1;
	//if(inflateInit2(&d_stream, 47) != Z_OK) 
	//return -1;
	while (d_stream.total_out < *ndata && d_stream.total_in < nzdata) {
		d_stream.avail_in = d_stream.avail_out = 1; /* force small buffers */
		if ((err = inflate(&d_stream, Z_NO_FLUSH)) == Z_STREAM_END)
			break;
		if (err != Z_OK)
		{
			if (err == Z_DATA_ERROR)
			{
				d_stream.next_in = (Bytef*)dummy_head;
				d_stream.avail_in = sizeof(dummy_head);
				if ((err = inflate(&d_stream, Z_NO_FLUSH)) != Z_OK)
				{
					return -1;
				}
			}
			else return -1;
		}
	}
	if (inflateEnd(&d_stream) != Z_OK)
		return -1;
	*ndata = d_stream.total_out;
	return 0;
}










void Compress::testcompress() {

	Compress compress;

	const char* data = "abcdefghigklmnopqrstuvwxyz";

	int datalen = lstrlenA(data);

	char dstbuf[0x1000];
	unsigned long dstsize = sizeof(dstbuf);

	int dstlen = compress.gzcompress((unsigned char*)data, datalen,(unsigned char*) dstbuf, &dstsize);

	char dstbuf2[0x1000];
	unsigned long dstsize2 = sizeof(dstbuf2);
	int dstlen2 = compress.gzdecompress((unsigned char*)dstbuf, dstsize, (unsigned char*)dstbuf2, &dstsize2);

	char dstbuf3[0x1000];
	unsigned long dstsize3 = sizeof(dstbuf3);
	int dstlen3 = compress.httpgzdecompress((unsigned char*)dstbuf, dstsize, (unsigned char*)dstbuf3, &dstsize3);


	char dstbuf4[0x1000];
	unsigned long dstsize4 = sizeof(dstbuf4);
	int dstlen4 = compress.zcompress((unsigned char*)data, datalen, (unsigned char*)dstbuf4, &dstsize4);

	char dstbuf5[0x1000];
	unsigned long dstsize5 = sizeof(dstbuf5);
	int dstlen5 = compress.zdecompress((unsigned char*)dstbuf4, dstsize4, (unsigned char*)dstbuf5, &dstsize5);
	return;
}


