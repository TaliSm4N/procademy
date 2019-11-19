#pragma comment(lib, "ws2_32")
#define UNICODE
#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <iostream>

BOOL DomainToIP(WCHAR *szDomain, IN_ADDR *pAddr)
{
	ADDRINFOW	*pAddrInfo;
	SOCKADDR_IN *pSockAddr;

	ADDRINFOW addrinfo;
	addrinfo.ai_family = AF_UNSPEC;
	addrinfo.ai_socktype = SOCK_STREAM;
	addrinfo.ai_protocol = IPPROTO_TCP;
	
	//if (GetAddrInfo(szDomain, L"0", (const ADDRINFOW *)&addrinfo, &pAddrInfo) != 0)
	if (GetAddrInfo(szDomain, L"0", NULL, &pAddrInfo) != 0)
	{
		std::cout << "fucking" << std::endl;
		return FALSE;
	}
	pSockAddr = (SOCKADDR_IN *)pAddrInfo->ai_addr;
	*pAddr = pSockAddr->sin_addr;
	FreeAddrInfo(pAddrInfo);
	
	return TRUE;
}

int main()
{
	WCHAR domain[128];
	IN_ADDR pAddr;

	char ip[100];

	WSADATA wsaData;

	WSAStartup(MAKEWORD(2, 2), &wsaData);

	std::wcout << L"Domain :";
	std::wcin >> domain;
	DomainToIP(domain, &pAddr);

	inet_ntop(AF_INET, &pAddr, ip, sizeof(ip));
	std::cout << ip << std::endl;
	system("pause");

	WSACleanup();
}