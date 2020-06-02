#pragma once


#define URL_MAX 1024

class CHTTP
{
public:
	CHTTP(int ConnectTimeout = 5000,int RecvTimeout = 5000, int SendTimeout=5000);

	bool SetURL(const WCHAR *url);
	bool HTTPPost(const WCHAR *path, const char *body, char *recv);
	bool HTTPURLPost(const WCHAR *url, const char *body, char *recv);
private:
	bool SetIP(char *url);
	SOCKET Connect();
	bool Send(SOCKET sock, const char *protocol);
	bool Recv(SOCKET sock, char *recvProtocol);
private:
	WSADATA _wsa;
	int _connectTimeout;
	int _recvTimeout;
	int _sendTimeout;
	char _url[URL_MAX];
	char _ip[100];
	int _port;
	IN_ADDR _addr;
};