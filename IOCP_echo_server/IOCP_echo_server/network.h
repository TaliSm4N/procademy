#pragma once


bool SendUnicast(Session* session, st_PACKET_HEADER& header, Packet& p);
bool SendUnicast(Session* session, Packet& p);