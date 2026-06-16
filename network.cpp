
#include <openssl/des.h>
#include <winsock2.h>
#include <Windows.h>
#include <string>
#include <iostream>
#include <Ws2tcpip.h>
#include "Include/zconf.h"
#include "Include/zlib.h"
#include <conio.h>
#include <stdio.h>
#include <intrin.h>



#pragma comment(lib,"ws2_32.lib")

using namespace std;


#pragma pack(1)



#pragma pack()



void printdata(char* data, int size) {
	for (int i = 0; i < size; i++) {
		printf("%02x ", (unsigned char)data[i]);
		if ((i + 1) % 16 == 0) {
			printf("\r\n");
		}
	}
}


int __stdcall UdpClient(string ip, int port, int direction) {

	int ret = 0;
	SOCKET s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (s == INVALID_SOCKET) {
		perror("socket error\r\n");
		return -1;
	}

	sockaddr_in sa = { 0 };
	sa.sin_family = AF_INET;
	sa.sin_addr.S_un.S_addr = inet_addr(ip.c_str());
	sa.sin_port = ntohs(port);

	char sendBuf[0x1000] = { 0 };
	int sendLen = sendto(s, sendBuf, 0x1000, 0, (sockaddr*)&s, sizeof(sockaddr_in));

	char recvbuf[0x1000];
	int addrlen = sizeof(sockaddr_in);
	sockaddr_in addr = { 0 };
	int recvlen = recvfrom(s, recvbuf, 0x1000, 0, (sockaddr*)&addr, &addrlen);
	if (recvlen <= 0) {
		perror("recvfrom error\r\n");
		return -1;
	}
	else {
		printf("recvfrom bytes:%d\r\n", recvlen);
	}
	closesocket(s);

	return 0;
}


int __stdcall UdpServer(int port, int direction) {

	int ret = 0;
	SOCKET s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (s == INVALID_SOCKET) {
		perror("socket error\r\n");
		return -1;
	}

	sockaddr_in sa = { 0 };
	sa.sin_family = AF_INET;
	sa.sin_addr.S_un.S_addr = INADDR_ANY;
	sa.sin_port = ntohs(port);
	ret = bind(s, (sockaddr*)&sa, sizeof(sockaddr_in));
	if (ret < 0) {
		perror("bind error\r\n");
		return -1;
	}
	printf("%s %d recvfrom port:%d\r\n", __FUNCTION__, __LINE__, port);
	//char sendBuf[0x1000];
	//int sendLen = sendto(s, sendBuf, 0x1000, 0, (sockaddr*)&s, sizeof(sockaddr_in));

	int recvbufsize = 0x10000;
	char* recvbuf = new char[recvbufsize];
	while (1) {
		int calen = sizeof(sockaddr_in);
		sockaddr_in ca = { 0 };
		int recvlen = recvfrom(s, recvbuf, recvbufsize, 0, (sockaddr*)&ca, &calen);
		if (recvlen <= 0) {
			perror("recvfrom error\r\n");
			return -1;
		}
		else {
			recvbuf[recvlen] = 0;
			char* strip = inet_ntoa(ca.sin_addr);
			int port = ntohs(ca.sin_port);
			printf("%s %d recv ip:%s port:%d bytes:%d,string:%s\r\n",
				__FUNCTION__, __LINE__, strip, ntohs(ca.sin_port), recvlen, recvbuf);

			printdata(recvbuf, recvlen);

			HANDLE hf = CreateFileA("udpsrv.data", GENERIC_WRITE, 0, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
			if (hf != INVALID_HANDLE_VALUE) {
				int fs = SetFilePointer(hf, 0, 0, FILE_END);
				DWORD cnt = 0;
				ret = WriteFile(hf, recvbuf, recvlen, &cnt, 0);
				CloseHandle(hf);
			}
		}
	}
	closesocket(s);

	return 0;
}


int __stdcall TcpClient(string ip, int port, int direction) {

	int ret = 0;

	SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
	if (s == INVALID_SOCKET) {
		perror("socket\r\n");
		return -1;
	}

	sockaddr_in sa = { 0 };
	sa.sin_family = AF_INET;
	sa.sin_addr.S_un.S_addr = inet_addr(ip.c_str());
	sa.sin_port = ntohs(port);

	ret = connect(s, (sockaddr*)&sa, sizeof(sockaddr_in));
	if (ret < 0) {
		perror("connect\r\n");
		return -1;
	}
	int bufsize = 0x100000;
	char* sendbuf = new char[bufsize];

	char* recvbuf = new char[bufsize];

	int recvLen = 0;
	recvLen = recv(s, recvbuf, bufsize - 1, 0);
	if (recvLen > 0 && recvLen < bufsize) {

	}

	closesocket(s);

	delete[] sendbuf;

	delete[] recvbuf;

	return 0;
}


int __stdcall TcpServer(int port, int direction) {

	int ret = 0;

	SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s == INVALID_SOCKET) {
		perror("socket error\r\n");
		return -1;
	}

	sockaddr_in sa = { 0 };
	sa.sin_family = AF_INET;
	sa.sin_addr.S_un.S_addr = 0;
	sa.sin_port = ntohs(port);

	ret = bind(s, (sockaddr*)&sa, sizeof(sockaddr_in));
	if (ret) {
		perror("bind\r\n");
		return -1;
	}

	ret = listen(s, 16);

	printf("%s %d listen port:%d\r\n", __FUNCTION__, __LINE__, port);

	char sendbuf[0x1000] = { 0 };

	int recvBufSize = 0x100000;
	char* recvbuf = new char[recvBufSize];
	const char* sh = "220 mx1.sina.com ESMTP Postfix\r\n";
	while (1) {
		sockaddr_in ca;
		int sasize = sizeof(sockaddr_in);
		SOCKET sc = accept(s, (sockaddr*)&ca, &sasize);
		if (sc != INVALID_SOCKET) {
			if (direction == 1) {
				strcpy(sendbuf, sh);
				int sendlen = send(sc, sendbuf, strlen(sh), 0);
				if (sendlen <= 0) {
					break;
				}
			}

			int recvlen = recv(sc, recvbuf, recvBufSize - 1, 0);
			if (recvlen <= 0 || recvlen >= recvBufSize) {
				memset(recvbuf, 0, recvBufSize);
				closesocket(sc);
			}
			else {
				recvbuf[recvlen] = 0;
				char* strip = inet_ntoa(ca.sin_addr);
				int port = ntohs(ca.sin_port);
				printf("%s %d recv ip:%s port:%d bytes:%d,string:%s\r\n",
					__FUNCTION__, __LINE__, strip, ntohs(ca.sin_port), recvlen, recvbuf);
				printdata(recvbuf, recvlen);

				HANDLE hf = CreateFileA("tcpsrv.data", GENERIC_WRITE, 0, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
				if (hf != INVALID_HANDLE_VALUE) {
					int fs = SetFilePointer(hf, 0, 0, FILE_END);
					DWORD cnt = 0;
					ret = WriteFile(hf, recvbuf, recvlen, &cnt, 0);
					CloseHandle(hf);
				}

				//int sendsize = send(sc, recvbuf, recvlen, 0);
				closesocket(sc);
			}
		}
		else {
			continue;
		}
	}
	return 0;
}




int Network(int argc, char** argv) {

	int ret = 0;

	if (argc < 4) {
		printf("example:%s server 443 tcp 0\r\n", argv[0]);
		printf("example:%s client 192.168.1.1 80 tcp 1\r\n", argv[0]);
		return -1;
	}

	WSADATA wsa = { 0 };
	ret = WSAStartup(0x0202, &wsa);
	if (ret) {
		perror("WSAStartup error\r\n");
		return -1;
	}

	char* mode = argv[0];
	if (lstrcmpiA(mode, "server") == 0) {
		if (argc < 4) {
			return -1;
		}
		int port = atoi(argv[1]);
		char* protocol = argv[2];
		int direction = atoi(argv[3]);
		
		if (_stricmp(protocol, "tcp") == 0) {
			TcpServer(port, direction);
		}
		else if (_stricmp(protocol, "udp") == 0) {
			UdpServer(port, direction);
		}
	}
	else if (lstrcmpiA(mode, "client") == 0) {
		if (argc < 5) {
			return -1;
		}
		char* ip = argv[1];
		int port = atoi(argv[2]);
		char* protocol = argv[3];
		int direction = atoi(argv[4]);
		printf("server ip:%s,port:%d protocol:%s\r\n", ip, port, protocol);	
		if (_stricmp(protocol, "tcp") == 0) {
			TcpClient(ip, port, direction);
		}
		else if (_stricmp(protocol, "udp") == 0) {
			UdpClient(ip, port, direction);
		}
	}
	else {
		return 0;
	}

	//printf("press any key to quit...\r\n");
	//ret = getchar();
	return 0;
}