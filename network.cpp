

#include <winsock2.h>
#include <Windows.h>
#include <string>
#include <iostream>


#include <Windows.h>
#include <Ws2tcpip.h>
#include "network.h"

using namespace std;

#pragma comment(lib,"ws2_32.lib")









int __stdcall TestUdpClient(string ip, int port) {

	int ret = 0;

	SOCKET s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (s == INVALID_SOCKET) {
		printf("%s %d error\r\n",__FUNCTION__,__LINE__);
		return -1;
	}

	sockaddr_in sa = { 0 };
	sa.sin_family = AF_INET;
	sa.sin_addr.S_un.S_addr = inet_addr(ip.c_str());
	sa.sin_port = ntohs(port);

	int sendsize = 0x1000;
	char* sendbuf = new char[sendsize];
	memset(sendbuf, 0, sendsize);

	ret = sendto(s, (char*)sendbuf, sendsize, 0, (sockaddr*)&sa, sizeof(sockaddr_in));

	char recvbuf[0x1000];
	int addrlen = sizeof(sockaddr_in);
	sockaddr_in saClient = { 0 };
	int recvlen = recvfrom(s, recvbuf, sizeof(recvbuf)-1, 0, (sockaddr*)&saClient, &addrlen);
	if (recvlen > 0) {
		recvbuf[recvlen] = 0;
		printf("recv data: %s\r\n", recvbuf);
	}
	closesocket(s);

	delete[] sendbuf;

	return 0;
}



int __stdcall TestTcpClient(string ip, int port) {

	int ret = 0;

	SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
	if (s == INVALID_SOCKET) {
		printf("%s %d error\r\n", __FUNCTION__, __LINE__);
		return -1;
	}

	sockaddr_in sa = { 0 };
	sa.sin_family = AF_INET;
	sa.sin_addr.S_un.S_addr = inet_addr(ip.c_str());
	sa.sin_port = ntohs(port);

	ret = connect(s, (sockaddr*)&sa, sizeof(sockaddr_in));
	if (ret < 0) {
		return -1;
	}
	int bufsize = 0x1000;
	char* sendbuf = new char[bufsize];
	memset(sendbuf, 0, bufsize);

	char* recvbuf = new char[bufsize];

	ret = send(s, (char*)sendbuf, bufsize, 0);

	int recvlen = recv(s, recvbuf, bufsize-1, 0);
	if (recvlen > 0) {
		recvbuf[recvlen] = 0;
		printf("recv data: %s\r\n", recvbuf);
	}
	
	closesocket(s);

	delete[] sendbuf;

	delete[] recvbuf;
	return 0;
}


int __stdcall TestTcpServer(int port) {

	int ret = 0;

	SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s == INVALID_SOCKET) {
		printf("%s %d error\r\n", __FUNCTION__, __LINE__);
		return -1;
	}

	sockaddr_in sa = { 0 };
	sa.sin_family = AF_INET;
	sa.sin_addr.S_un.S_addr = 0;
	sa.sin_port = ntohs(port);

	ret = bind(s, (sockaddr*)&sa, sizeof(sockaddr_in));

	ret = listen(s, 16);

	unsigned int recvsize = 0x1000;
	char* recvbuf = new char[recvsize];

	while (1) {
		sockaddr_in client;
		int casize = sizeof(sockaddr_in);
		SOCKET sc = accept(s, (sockaddr*)&client, &casize);
		if (sc != INVALID_SOCKET) {

			int recvlen = recv(sc, recvbuf, recvsize-1, 0);
			if (recvlen <= 0) {
				closesocket(sc);
			}
			else {
				recvbuf[recvlen] = 0;
				const char* html = "hello how are you?\r\n";
				int sendsize = send(sc, html, lstrlenA(html), 0);
				closesocket(sc);
				printf("recv msg:\r\n");
				printf(recvbuf);
			}
		}
		else {
			continue;
		}
	}
	return 0;
}

int __stdcall TestUdpServer(int port) {

	int ret = 0;

	SOCKET s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (s == INVALID_SOCKET) {
		printf("%s %d error\r\n", __FUNCTION__, __LINE__);
		return -1;
	}

	sockaddr_in sa = { 0 };
	sa.sin_family = AF_INET;
	sa.sin_addr.S_un.S_addr = 0;
	sa.sin_port = ntohs(port);

	ret = bind(s, (sockaddr*)&sa, sizeof(sockaddr_in));

	unsigned int recvsize = 0x1000;
	char* recvbuf = new char[recvsize];

	while (1) {
		sockaddr_in client;
		int casize = sizeof(sockaddr_in);

		int recvlen = recvfrom(s, recvbuf, recvsize - 1, 0, (sockaddr*)&client,&casize);
		if (recvlen > 0) {
			recvbuf[recvlen] = 0;
			const char* html = "hello how are you?\r\n";
			int sendsize = sendto(s, html, lstrlenA(html), 0, (sockaddr*)&client, sizeof(sockaddr_in));
			closesocket(s);
			printf("recv msg:\r\n");
			printf(recvbuf);
		}
	}
	return 0;
}


int TestNetwork( char* strip,char * strport,char * protocol) {

	int ret = 0;
	WSADATA wsa = { 0 };
	ret = WSAStartup(0x0202, &wsa);
	if (ret) {
		printf("%s %d error\r\n", __FUNCTION__, __LINE__);
		return -1;
	}

	int port = atoi(strport);

	printf("ip:%s,port:%d\r\n", strip, port);

	int opt = atoi(protocol);

	if (opt == 6) {
		TestTcpClient(strip, port);
	}
	else if (opt == 17) {
		TestUdpClient(strip, port);
	}

	return 0;
}