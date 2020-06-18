#pragma once

#pragma pack(push,1)
typedef struct NetServerHeader
{
	BYTE code;
	WORD len;
	BYTE RandKey;
	BYTE CheckSum;

}HEADER;
#pragma pack(pop)