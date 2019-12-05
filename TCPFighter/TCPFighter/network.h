#pragma once


#include <Windows.h>
#include "PacketDefine.h"
#include "RingBuffer.h"
#include "Packet.h"

bool networkInit(HWND hWnd, int WM_SOCKET);
void ProcessSocketMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void ProcRead(HWND hWnd);
void ProcReadDirect(HWND hWnd);
void PacketProc(BYTE byPacketType, char *Packet);
void PacketProc(BYTE byPacketType, Packet &p);
void ProcWrite();
void ProcWriteDirect();
void SendPacket(st_NETWORK_PACKET_HEADER *pHeader, char *pPacket);
void SendPacket(st_NETWORK_PACKET_HEADER *pHeader, Packet &p);

void SendAttack(BYTE atkType, BYTE dir, WORD x, WORD y);
void SendMove(BYTE dir, WORD x, WORD y);
void SendStand(BYTE dir, WORD x, WORD y);