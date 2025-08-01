#pragma once




#define PCAP_OPEN_LIVE_TO_MS_VALUE_NEGTIVE	-1
#define PCAP_OPEN_LIVE_TO_MS_VALUE_0		0
#define WINPCAP_MAX_BUFFER_SIZE				0x400000
#define PCAP_OPENFLAG_PROMISCUOUS			1
#define WINPCAP_MAX_PACKET_SIZE				0x10000	
//#define MAX_IPPACKET_SIZE					1514
#define PCAP_MAIN_FILTER			"tcp or udp"			
//#define PCAP_PORT_FILTER			"tcp dst port 80 or tcp dst port 8080 or tcp dst port 8000 or udp dst port 53"	
#define PCAP_FILTER_MASK_VALUE		0xffffff

#define WINPCAP_NETCARD_NAME_PREFIX			"\\Device\\NPF_"

struct SOCKET_OBJECT {

	unsigned long localIP;
	unsigned long remoteIP;
	unsigned short localPort;
	unsigned short remotePort;
	unsigned char protocol;


};


int NetworkProxy(char* srcIP, char* proxyIP);