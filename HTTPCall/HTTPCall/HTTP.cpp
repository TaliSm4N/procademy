//test
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>

#include <WinSock2.h>
//#include <windows.h>
#include <WS2tcpip.h>

#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "ws2_32")

#include "HTTP.h"

void WCharToChar(const WCHAR *wChr, char *chr)
{
	int len = wcslen(wChr) + 1;

	wcstombs_s(NULL, chr, len, wChr, len);
}

void CharToWChar(const char *chr, WCHAR *wChr)
{
	int len = strlen(chr) + 1;

	mbstowcs_s(NULL, wChr, len, chr, len);
}

CHTTP::CHTTP(int ConnectTimeout, int RecvTimeout, int SendTimeout)
	:_connectTimeout(ConnectTimeout),_recvTimeout(RecvTimeout),_sendTimeout(SendTimeout),_port(80)
{
	WSAStartup(MAKEWORD(2, 2), &_wsa);
}

bool CHTTP::SetURL(const WCHAR *url)
{
	char str[URL_MAX];
	char *temp = str;
	WCharToChar(url, temp);

	char *ptr=strstr(temp, "http://");

	if (ptr != NULL)
	{
		temp = temp + 7;
	}

	ptr = strchr(temp, ':');
	
	if (ptr != NULL)
	{
		*ptr = '\0';
		ptr++;
		_port = 0;

		while (1)
		{
			if (*ptr >= '0'&&*ptr <= '9')
			{
				_port *= 10;
				_port += *ptr - 48;
			}
			else if (*ptr == '/'||*ptr=='\n'||*ptr=='\0')
				break;
			else
				return false;
			ptr++;
		}
	}
	
	ptr = strchr(temp, '/');

	if (ptr != NULL)
		*ptr = '\0';

	strcpy_s(_url, temp);

	return SetIP(_url);
}

bool CHTTP::SetIP(char *url)
{
	char port[10];
	ADDRINFOA hints;
	ADDRINFOA *servinfo;
	_itoa(_port,port,10);
	ZeroMemory(&hints, sizeof(hints));

	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;


	if (GetAddrInfoA(url, port, &hints, &servinfo) != 0)
	{
		return false;
	}

	_addr = ((SOCKADDR_IN *)servinfo->ai_addr)->sin_addr;
	inet_ntop(AF_INET, &_addr, _ip, sizeof(_ip));

	if (strcmp(_ip, "0.0.0.0") == 0)
		strcpy_s(_ip, "127.0.0.1");

	FreeAddrInfoA(servinfo);

	return true;
}

bool CHTTP::HTTPPost(const WCHAR *path, const char *body, char *recvProtocol)
{
	SOCKET sock = Connect();

	if (sock == INVALID_SOCKET)
		return false;

	char cpath[100];
	WCharToChar(path, cpath);

	char protocol[2000];

	sprintf(protocol, "POST /%s HTTP/1.1\r\nUser-Agent: XX\r\nHost:%s\r\nConnection: Close\r\nContent-Length:%d\r\n\r\n%s", cpath,_url, strlen(body),body);

	printf("%s\n",protocol);

	if (!Send(sock, protocol))
		return false;

	if (!Recv(sock, recvProtocol))
		return false;

	return true;
}

SOCKET CHTTP::Connect()
{
	SOCKET sock;
	SOCKADDR_IN addr;
	ZeroMemory(&addr, sizeof(addr));

	fd_set wset;
	struct timeval  tval;

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0)
		return INVALID_SOCKET;

	u_long on = 1;
	if (ioctlsocket(sock, FIONBIO, &on) == SOCKET_ERROR)
	{
		closesocket(sock);
		return INVALID_SOCKET;
	}



	if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&_recvTimeout, sizeof(_recvTimeout)) == SOCKET_ERROR)
	{
		closesocket(sock);
			return INVALID_SOCKET;
	}


	if (setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char *)&_sendTimeout, sizeof(_sendTimeout)) == SOCKET_ERROR)
	{
		closesocket(sock);
		return INVALID_SOCKET;
	}

	addr.sin_family = AF_INET;
	inet_pton(AF_INET, _ip, &addr.sin_addr.s_addr);
	addr.sin_port = htons(_port);




	if (connect(sock, (SOCKADDR *)&addr, sizeof(addr)) == SOCKET_ERROR)
	{
		if (GetLastError() != WSAEWOULDBLOCK)
		{
			closesocket(sock);
			return INVALID_SOCKET;
		}
	}

	FD_ZERO(&wset);
	FD_SET(sock, &wset);
	tval.tv_sec = _connectTimeout / 1000;
	tval.tv_usec = _connectTimeout % 1000;

	int result = select(0, NULL, &wset, NULL, &tval);
	if (result > 0)
	{
		if (FD_ISSET(sock, &wset))
		{
			on = 0;
			if (ioctlsocket(sock, FIONBIO, &on) == SOCKET_ERROR)
			{
				closesocket(sock);
				return INVALID_SOCKET;
			}
		}
	}
	else if (result == SOCKET_ERROR)
	{
		closesocket(sock);
		return INVALID_SOCKET;
	}
	else if (result == 0)
	{
		closesocket(sock);
		return INVALID_SOCKET;
	}

	return sock;
}

bool CHTTP::Send(SOCKET sock, const char *protocol)
{
	int sendSize = send(sock, protocol, strlen(protocol), 0);
	if(sendSize == SOCKET_ERROR)
	{
		closesocket(sock);
		return false;	
	}
	else if (sendSize != strlen(protocol))
	{
		if (sendSize > strlen(protocol))
		{
			closesocket(sock);
			return false;
		}

		return Send(sock, protocol + sendSize);
	}


	
	return true;
}

bool CHTTP::Recv(SOCKET sock, char *recvProtocol)
{
	bool header = true;
	bool first = true;
	bool conlen = true;
	bool getlen = true;
	char buf[1500];
	char *cur;
	char *recvCur=recvProtocol;
	int length=0;

	while (1)
	{
		int recvSize = recv(sock, buf, 1500, 0);
		if (recvSize == SOCKET_ERROR)
		{
			closesocket(sock);
			return false;
		}
		buf[recvSize] = '\0';
		cur = buf;

		if (header)
		{
			if (first)
			{
				if (strstr(buf, "HTTP/1.1 200 OK") == buf)
				{
					first = false;
				}
			}

			if (conlen)
			{
				cur = strstr(buf, "Content-Length: ");

				if (cur == NULL)
					continue;
				cur += 16;
				conlen = false;
			}

			while (getlen)
			{
				length *= 10;
				length += *cur - 48;

				cur++;

				if (*cur == '\r'&& *(cur+1) == '\n')
				{
					getlen = false;
					break;
				}

				if (*cur == '\0')
					break;

			}

			cur = strstr(cur, "\r\n\r\n");

			if (cur == NULL)
				continue;

			cur += 4;
			header = false;
		}
		strcpy(recvCur, cur);

		recvCur += strlen(cur);
		length -= strlen(cur);

		if (length == 0)
		{
			break;
		}
	}
	



	return true;
}