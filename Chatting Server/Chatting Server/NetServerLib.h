#pragma once


//#include <sdkddkver.h>
#include <WinSock2.h>
//#include <windows.h>
#include <WS2tcpip.h>
#include <process.h>
#include <fstream>
#include <cstring>
#include <string>
#include <map>
#include <unordered_map>
#include <stack>

#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "ws2_32")

#include "Log.h"
#include "CrashDump.h"
#include "MemoryPool.h"
#include "MemoryPoolTLS.h"
#include "LockFreeStack.h"
#include "LockFreeQueue.h"
#include "Packet.h"
#include "PacketPtr.h"
#include "RingBuffer.h"
#include "Session.h"
#include "TextParser.h"
#include "NetServer.h"

