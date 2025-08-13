#pragma once

#include <string>

using namespace std;

int __stdcall TestUdpClient(string ip, int port);

int __stdcall TestTcpClient(string ip, int port);

int __stdcall TestUdpServer(int port);

int __stdcall TestTcpServer(int port);

int TestNetwork(char* strip, char* strport, char* protocol);