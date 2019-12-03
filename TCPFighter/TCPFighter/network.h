#pragma once


#include <Windows.h>
#include "PacketDefine.h"
#include "RingBuffer.h"

bool networkInit(HWND hWnd, int WM_SOCKET);
void ProcessSocketMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void ProcRead(HWND hWnd);
void ProcReadDirect(HWND hWnd);
void PacketProc(BYTE byPacketType, char *Packet);
void ProcWrite();
void ProcWriteDirect();
void SendPacket(st_NETWORK_PACKET_HEADER *pHeader, char *pPacket);