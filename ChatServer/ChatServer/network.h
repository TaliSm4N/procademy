#pragma once
#pragma comment(lib,"ws2_32")
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <ws2tcpip.h>
#include "info.h"
#include "Protocol.h"

bool initNetwork(int PORT);
bool networkProcess();
bool callSelect(DWORD* IDTable, SOCKET* sockTable, FD_SET* ReadSet, FD_SET* WriteSet);
bool procAccept();
bool procSend(DWORD userID);
bool procRecv(DWORD userID);
UserInfo* FindClient(DWORD userID);
RoomInfo* FindRoom(DWORD roomID);
bool SendUnicast(UserInfo* client, st_PACKET_HEADER& header, Packet& p);
bool SendBroadCast(UserInfo* client, st_PACKET_HEADER& header, Packet& p);
bool SendBroadCastAll(UserInfo* client, st_PACKET_HEADER& header, Packet& p);
bool SendBroadCast_Room(UserInfo* client, RoomInfo *room, st_PACKET_HEADER& header, Packet& p);
bool SendBroadCastAll_Room(UserInfo* client, RoomInfo *room, st_PACKET_HEADER& header, Packet& p);
bool recvLogin(UserInfo* client, Packet& p);
bool recvEnterRoom(UserInfo* client, Packet& p);
bool recvRoomList(UserInfo* client);
bool recvMakeRoom(UserInfo* client, Packet& p,WORD len);
bool recvRoomLeave(UserInfo* client);
bool recvChat(UserInfo* client, Packet& p);
bool sendLogin(UserInfo* client, BYTE result, DWORD ID);
bool sendEnterRoom(UserInfo *client, BYTE result, RoomInfo *room);
bool sendRoomList(UserInfo *client);
bool sendMakeRoom(UserInfo *client, BYTE result, RoomInfo *room);
bool sendRoomLeave(UserInfo *client);
bool sendChat(UserInfo *client, RoomInfo *room, WCHAR *msg, WORD len);
BYTE setCheckSum(Packet &p, WORD type);
bool getCheckSum(st_PACKET_HEADER &header, Packet &p);