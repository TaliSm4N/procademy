#pragma once

extern std::map<LONGLONG, Session *> sessionList;
extern SRWLOCK sessionListLock;

unsigned int WINAPI WorkerThread(LPVOID lpParam);
unsigned int WINAPI AcceptThread(LPVOID lpParam);