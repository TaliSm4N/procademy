#pragma once

extern std::map<LONGLONG, Session *> sessionList;

void OnRecv(LONGLONG sessionID, Packet &p,int type = 0);

void OnClientLeave(LONGLONG sessionID);

void OnClientJoin(LONGLONG sessionID);

void InitContents();