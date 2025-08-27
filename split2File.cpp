
#include <Windows.h>
#include "utils.h"


#include "Split2File.h"
#include "FileUtils.h"

// "\x7f\x45\x4c\x46"
int Split2File(char* data,int size,char * path,unsigned char * tag,int taglen) {

	int ret = 0;

	char ofn[1024];

	__int64 num = (__int64)0;

	char newpath[1024] = { 0 };
	newpath[0] = '.';
	newpath[1] = '\\';
	lstrcatA(newpath, path);
	ret = CreateDirectoryA(newpath,0);

	char* hdr = data;
	char* end = hdr;
	while ( end < data + size) {
		if (end != hdr) {
			if (memcmp(hdr, tag, taglen) == 0 && memcmp(end, tag, taglen) == 0) {

				wsprintfA(ofn, ".\\%s\\%08I64d", path, num);
				num++;

				ret = FWriter(ofn, hdr, end - hdr, 0);

				hdr = end;
			}
			else if (memcmp(end, tag, taglen) == 0) {
				hdr = end;
			}
		}
		else {

		}
		
		end ++;
	}

	if (memcmp(hdr, tag, taglen) == 0) {
		wsprintfA(ofn, ".\\%s\\%08I64d", path, num);
		num++;

		FWriter(ofn, hdr, end - hdr, 0);

		hdr = end;
	}

	return num;
}