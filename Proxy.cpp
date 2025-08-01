
#define WIN32_LEAN_AND_MEAN  // 减少无关头文件
#include <winsock2.h>
#include <ws2tcpip.h>  // 新 API（如 inet_pton）

#include <Windows.h>
#include <string>
#include "Proxy.h"
#include <Iptypes.h >
#include <iphlpapi.h>
#include "utils.h"
#include "FileUtils.h"
#include "Packet.h"

#include<map>
#include<unordered_map>


#include "include\\pcap.h"
#include "include\\pcap\\pcap.h"

using namespace std;


#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "Iphlpapi.lib") 
#pragma comment(lib,"wpcap.lib")


#define MAC_ADDRESS_SIZE					6

std::unordered_map<char*,struct SOCKET_OBJECT * > g_map;


unsigned long g_ProxyIP = 0;

unsigned long g_SourceIP = 0;

string GetAdapterAlias(string adaptername) {
	unsigned char szalias[MAX_PATH] = { 0 };
	//can not be \\SYSTEM,why?????
	string subkey = 
		"SYSTEM\\CurrentControlSet\\Control\\Network\\{4D36E972-E325-11CE-BFC1-08002BE10318}\\" + adaptername + "\\Connection\\";

	int ret = QueryRegistryValue(HKEY_LOCAL_MACHINE, (char*)subkey.c_str(),REG_SZ,(char*) "Name", szalias);
	if (ret)
	{
		return string((char*)szalias);
	}

	return "";
}

PIP_ADAPTER_INFO GetNetCardAdapter(PIP_ADAPTER_INFO pAdapterInfo, int seq) {

	PIP_ADAPTER_INFO pAdapter = pAdapterInfo;
	for (int number = 0; number < seq; pAdapter = pAdapter->Next, number++)
	{
		if (pAdapter == NULL)
		{
			return FALSE;
		}
	}
	return pAdapter;
}

PIP_ADAPTER_INFO ShowNetCardInfo(int* count) {
	ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);
	PIP_ADAPTER_INFO pAdapterInfo = (IP_ADAPTER_INFO*)GlobalAlloc(GPTR, sizeof(IP_ADAPTER_INFO));
	if (pAdapterInfo == NULL)
	{
		printf("%s GlobalAlloc error\r\n",__FUNCTION__);
		return FALSE;
	}

	if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW)
	{
		GlobalFree((char*)pAdapterInfo);
		pAdapterInfo = (IP_ADAPTER_INFO*)GlobalAlloc(GPTR, ulOutBufLen);
		if (pAdapterInfo == NULL)
		{
			printf("%s GlobalAlloc error\r\n", __FUNCTION__);
			return FALSE;
		}
	}

	if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == NO_ERROR)
	{
		int number = 0;
		PIP_ADAPTER_INFO pAdapter = 0;
		printf("本机安装的网卡列表如下:\r\n");
		for (pAdapter = pAdapterInfo; pAdapter != NULL; pAdapter = pAdapter->Next)
		{
			number++;

			string aliasname = GetAdapterAlias(pAdapter->AdapterName);

			printf("网卡号码:\t%d\r\n网卡名称:\t%s\r\n网卡别名:\t%s\r\n网卡描述:\t%s\r\n网卡类型:\t%d\r\n网卡IP地址:\t%s\r\n网关IP地址:\t%s\r\n\r\n",
				number, pAdapter->AdapterName, aliasname.c_str(), pAdapter->Description, pAdapter->Type, pAdapter->IpAddressList.IpAddress.String,
				pAdapter->GatewayList.IpAddress.String);
		}

		*count = number;
		return pAdapterInfo;
	}
	else
	{
		printf("%s GetAdaptersInfo error\r\n", __FUNCTION__);
		GlobalFree((char*)pAdapterInfo);
		return FALSE;
	}
}


string SelectNetcard(unsigned long* localIP, unsigned long* netmask, unsigned long* netgateip, unsigned char* lplocalmac, int& selectedcard) {
	int	iInterfaceCnt = 0;
	PIP_ADAPTER_INFO padpterInfo = ShowNetCardInfo(&iInterfaceCnt);
	if (padpterInfo == FALSE)
	{
		return "";
	}

	if (selectedcard == -1)
	{
		printf("%s(1-%d):", "请选择抓包网卡序号", iInterfaceCnt);
		scanf_s("%d", &selectedcard);
		printf("\n");
	}

	if (selectedcard < 1 || selectedcard > iInterfaceCnt)
	{
		printf("Interface number out of range\n");
		return "";
	}

	PIP_ADAPTER_INFO pAdapter = GetNetCardAdapter(padpterInfo, selectedcard - 1);

	string adaptername = pAdapter->AdapterName;
	*localIP = inet_addr(pAdapter->IpAddressList.IpAddress.String);
	*netmask = inet_addr(pAdapter->IpAddressList.IpMask.String);
	*netgateip = inet_addr(pAdapter->GatewayList.IpAddress.String);
	memmove(lplocalmac, pAdapter->Address, MAC_ADDRESS_SIZE);
	string aliasname = GetAdapterAlias(adaptername);

	GlobalFree((char*)padpterInfo);

	return adaptername;
}


pcap_t* InitPcap(string adaptername,int delay) {

		int ret = 0;

		string devname = string(WINPCAP_NETCARD_NAME_PREFIX) + adaptername;

		char errBuf[PCAP_ERRBUF_SIZE * 2] = { 0 };
		pcap_t* pcapt = pcap_open_live(devname.c_str(), WINPCAP_MAX_PACKET_SIZE, PCAP_OPENFLAG_PROMISCUOUS, delay, errBuf);
		if (pcapt == NULL)
		{
			fprintf(stderr, "\nUnable to open the adapter. %s is not supported by WinPcap，error:%s\n", devname.c_str(), errBuf);
			return 0;
		}

		ret = pcap_setbuff(pcapt, WINPCAP_MAX_BUFFER_SIZE);	//the limit buffer size of capraw is 100M
		if (ret == -1)
		{
			printf("pcap_setbuff size:%d error:%s\n", WINPCAP_MAX_BUFFER_SIZE, errBuf);
			return FALSE;
		}

		//statistics return only data stripped mac,ip,tcp or udp header left user data
	// 	ret = pcap_setmode(pcapt, MODE_STAT);

	// 	bpf_program		stBpfp = { 0 };
	// 	ret = pcap_compile(pcapt, &stBpfp, PCAP_MAIN_FILTER, TRUE, netmask);
	// 	if (ret < 0)
	// 	{
	// 		fprintf(stderr, "数据包过滤条件语法设置失败,请检查过滤条件的语法设置\n");
	// 		getch();
	// 		return FALSE;
	// 	}
	// 
	// 	ret = pcap_setfilter(pcapt, &stBpfp);
	// 	if (ret < 0)
	// 	{
	// 		fprintf(stderr, "数据包过滤条件设置失败\n");
	// 		getch();
	// 		return FALSE;
	// 	}

		return pcapt;
	
}



int GetIPHeader(LPMACHEADER mac, LPPPPOEHEADER& pppoe, LPIPHEADER& ip, LPIPV6HEADER& ipv6) {
	char* nexthdr = (char*)mac + sizeof(MACHEADER);
	int nextprotocol = mac->Protocol;
	if (nextprotocol == 0x0081)
	{
		LPHEADER8021Q p8021q = (LPHEADER8021Q)nexthdr;
		if (p8021q->type == 0x0081)
		{
			LPHEADER8021Q p8021q2 = LPHEADER8021Q((char*)p8021q + sizeof(HEADER8021Q));

			nexthdr = (char*)p8021q2 + (sizeof(HEADER8021Q));

			nextprotocol = p8021q2->type;
		}
		else {
			nexthdr = (char*)p8021q + sizeof(HEADER8021Q);

			nextprotocol = p8021q->type;
		}
	}
	else if (nextprotocol == 0x9899 || nextprotocol == 0xa788 || nextprotocol == 0xcc88)
	{
		return 0;
	}

	//0x8863（Discovery阶段或拆链阶段）或者0x8864（Session阶段）
	if (nextprotocol == 0x6488)
	{
		//0×C021 LCP数据报文
		//0×8021 NCP数据报文
		//0×0021 IP数据报文

		pppoe = (LPPPPOEHEADER)nexthdr;
		nextprotocol = pppoe->protocol;
		if (nextprotocol == 0x2100)
		{
			nexthdr = (char*)pppoe + sizeof(PPPOEHEADER);

			ip = (LPIPHEADER)nexthdr;

			return 1;
		}
		else if (nextprotocol == 0x5700 || nextprotocol == 0x5780) //ipv6
		{
			nexthdr = (char*)pppoe + sizeof(PPPOEHEADER);

			ipv6 = (LPIPV6HEADER)nexthdr;

			return 2;
		}
		else if (nextprotocol == 0x21c0 ||
			nextprotocol == 0x0101 ||
			nextprotocol == 0x23c0 ||
			nextprotocol == 0x22c0 ||
			nextprotocol == 0x2180)
		{
			return 0;
		}
		else {
			return -1;
		}
	}
	else if (nextprotocol == 0x0008)
	{
		ip = (LPIPHEADER)nexthdr;
		return 1;
	}
	else if (nextprotocol == 0xdd86)
	{
		ipv6 = (LPIPV6HEADER)nexthdr;

		return 2;
	}
	//0x2700 IEEE 802.3
	else if (nextprotocol == 0x0608 || nextprotocol == 0x6388 || nextprotocol == 0x2700)
	{
		return 0;
	}
	else {
		return -1;
	}
}




int GenKey(unsigned long srcip,unsigned long dstip,unsigned short srcport,unsigned short dstport,int protocol,char * buf) {
	int len = wsprintfA(buf, "%08X_%08X_%04X_%04X_%02X", srcip, dstip, srcport, dstport,protocol);
	return len;
}

int insertMap(char * key,char * value) {
	return 0;

}


void traversalMap() {
	auto iter = g_map.begin();
	while (iter != g_map.end()) {
		iter++;
	}
}

int searchMap(char* key) {

	auto it = g_map.find(key);
	if (it == g_map.end()) {
		return TRUE;
	}
	else {
		return 0;
	}
}

int removeMap() {
	return 0;
}

int ProcessPacket(char* data, int size) {

	LPPPPOEHEADER pppoe = 0;
	LPIPHEADER pIPV4Hdr = 0;
	LPIPV6HEADER pIPV6 = 0;
	LPMACHEADER pMac = (LPMACHEADER)data;
	int iptype = GetIPHeader(pMac, pppoe, pIPV4Hdr, pIPV6);
	if (iptype <= 0)
	{
		return -1;
	}
	else if (iptype == 1)
	{

	}
	else if (iptype == 2) {

	}

	if (pIPV4Hdr->SrcIP == g_SourceIP) {

		char key[256];

		int iIpv4HdrLen = pIPV4Hdr->HeaderSize << 2;
		if (pIPV4Hdr->Protocol == IPPROTO_TCP)
		{
			LPTCPHEADER pTcpHdr = (LPTCPHEADER)((char*)pIPV4Hdr + iIpv4HdrLen);
			GenKey(0, pIPV4Hdr->DstIP, pTcpHdr->SrcPort, pTcpHdr->DstPort, pIPV4Hdr->Protocol, key);
		}
		else if (pIPV4Hdr->Protocol == IPPROTO_UDP) {
			LPUDPHEADER pUdpHdr = (LPUDPHEADER)((char*)pIPV4Hdr + iIpv4HdrLen);
			GenKey(0, pIPV4Hdr->DstIP, pUdpHdr->SrcPort, pUdpHdr->DstPort, pIPV4Hdr->Protocol, key);
		}
		else {

		}
		auto it = g_map.find(key);
		if (it == g_map.end()) {

		}
		else {
			char* data = new char[sizeof(SOCKET_OBJECT)];
			struct SOCKET_OBJECT* obj = (SOCKET_OBJECT*)data;
			g_map.insert(std::make_pair(key, obj));
		}

	}

	return 0;
}


int GetMACFromIP(unsigned long remoteIP,unsigned long localIP, unsigned char mac[]) {
	unsigned long dwMacLen = MAC_ADDRESS_SIZE;
	int nRetCode = SendARP(remoteIP, localIP, (unsigned long*)mac, &dwMacLen);
	if (nRetCode != NO_ERROR)
	{
		return 0;
	}
	return TRUE;
}

int NetworkProxy(char * srcIP,char* proxyIP) {
	int ret = 0;

	g_SourceIP = inet_addr(srcIP);
	g_ProxyIP = inet_addr(proxyIP);

	WORD wVersionRequested = MAKEWORD(2, 2);
	WSADATA wsaData = { 0 };
	if (WSAStartup(wVersionRequested, &wsaData) != 0)
	{
		printf("WSAStartup error,error code is:%d\n", GetLastError());
		getchar();
		return -1;
	}

	unsigned long localIP = 0;
	unsigned long localPort = 0;
	unsigned long netMask = 0;
	unsigned long gateway = 0;
	unsigned char localMac[6] = { 0 };

	int cardNum = -1;

	string netcard = SelectNetcard(&localIP, &netMask, &gateway, localMac, cardNum);

	pcap_t *pcap = InitPcap(netcard, -1);

	pcap_pkthdr* pHeader = 0;
	const char* pData = 0;

	while (TRUE)
	{
		ret = pcap_next_ex(pcap, &pHeader, (const unsigned char**)&pData);
		int iCapLen = pHeader->len;
		if (ret == 0)
		{
			continue;
		}
		else if (ret < 0)
		{
			printf("pcap_next_ex error:%s,return value:%d\r\n", pcap_geterr(pcap), ret);
			continue;
		}
		else if (iCapLen >= WINPCAP_MAX_PACKET_SIZE || iCapLen <= 0)
		{
			printf("pcap_next_ex error:%s,packet caplen:%u or len:%u error\r\n", 
				pcap_geterr(pcap), pHeader->caplen, pHeader->len);
			continue;
		}

		*((DWORD*)pData + iCapLen) = 0;

		ret = ProcessPacket((char*)pData, iCapLen);
	}


	return TRUE;
}