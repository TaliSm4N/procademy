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

bool recvAccountList(LoginUser *client);
bool sendAccountList(LoginUser *client);

bool recvFriendList(LoginUser *client);
bool sendFriendList(LoginUser *client);

bool recvRequestList(LoginUser *client);
bool sendRequestList(LoginUser *client);

bool recvReplyList(LoginUser *client);
bool sendReplyList(LoginUser *client);

bool recvRemove(LoginUser *client, Packet &p);
bool sendRemove(LoginUser *client, UINT64 accountNo, BYTE result);

bool recvRequest(LoginUser *client, Packet &p);
bool sendRequest(LoginUser *client, UINT64 accountNo, BYTE result);

bool recvRequestCancel(LoginUser *client, Packet &p);
bool sendRequestCancel(LoginUser *client, UINT64 accountNo, BYTE result);

bool recvRequestDeny(LoginUser *client, Packet &p);
bool sendRequestDeny(LoginUser *client, UINT64 accountNo, BYTE result);

bool recvAgree(LoginUser *client, Packet &p);
bool sendAgree(LoginUser *client, UINT64 accountNo, BYTE result);

bool recvStress(LoginUser *client, Packet &p);
bool sendStress(LoginUser *client, Packet &p);