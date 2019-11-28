#pragma once


#include <Windows.h>
#include "RingBuffer.h"

bool networkInit(HWND hWnd, int WM_SOCKET);
void ProcessSocketMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void ProcRead(HWND hWnd);
void ProcWrite();