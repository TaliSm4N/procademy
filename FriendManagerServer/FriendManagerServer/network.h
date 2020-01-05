#pragma once
#pragma comment(lib,"ws2_32")
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <ws2tcpip.h>
#include "Info.h"

bool initNetwork();
bool networkProcess();
bool callSelect(int* IDTable, SOCKET* sockTable, FD_SET* ReadSet, FD_SET* WriteSet);
bool procAccept();
bool procSend(DWORD userID);
bool procRecv(DWORD userID);

LoginUser *FindClient(int id);
Account *FindAccount(UINT64 id);

bool recvAccountADD(LoginUser *client,Packet &p);
bool sendAccountADD(LoginUser *client, UINT64 accountNo);

bool recvLogin(LoginUser *client, Packet &p);
bool sendLogin(LoginUser *client);