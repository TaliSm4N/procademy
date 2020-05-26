#include <iostream>
#include <Windows.h>
#include <stdio.h>

#include "header.h"
#include "MemoryPool.h"
#include "MemoryPoolTLS.h"
#include "Packet.h"

char semi[55];
char tempRan[55];
char tempKey[55];

void encode(Packet *payload,int ranKey)
{
	char temp[1000];
	char key = 0xa9;
	int len = payload->GetDataSize();
	payload->GetData(temp, len);
	char p=0;
	
	tempRan[0] = (ranKey + p + 1);
	p = temp[0] ^ (ranKey + p + 1);
	semi[0] = p;
	tempKey[0] = (key + 1);
	temp[0] = p ^ (key + 1);

	for (int i = 1; i < len; i++)
	{
		tempRan[i] = (ranKey + p + i + 1);
		p = temp[i] ^ (ranKey + p + i + 1);
		semi[i] = p;
		tempKey[i] = (key + i + 1 + temp[i - 1]);
		temp[i] = p ^ (key + i + 1 + temp[i - 1]);
	}
	payload->Clear();
	payload->PutData(temp, len);
}

void decode(Packet *payload, int ranKey)
{
	char temp[1000];
	char key = 0xa9;
	int len = payload->GetDataSize();
	payload->GetData(temp, len);
	char p= 0;
	char before_p = 0;
	char before_e = 0;

	p = temp[0] ^ (key + 1);
	before_p = p;
	before_e = temp[0];
	temp[0] = p ^ (ranKey + 1);

	//여기 문제
	for (int i = 1; i < len; i++)
	{
		
		before_p = p;
		p = temp[i] ^ (before_e + key + i + 1);
		before_e = temp[i];
		temp[i] = p ^ (before_p + ranKey + i + 1);
	}
	//여기 문제
	
	payload->Clear();
	payload->PutData(temp, len);
}

int main()
{
	Packet::Init(0x32, 0x77);
	Packet *p = Packet::Alloc();

	char testData[55] = "aaaaaaaaaabbbbbbbbbbcccccccccc1234567890abcdefghijklmn";
	char temp[55];

	//char test[55] = { 0xf9, 0x43, 0x95, 0x8c, 0x5f, 0xf3, 0xf7, 0x44, 0xb1, 0x87, 0x46, 0x23, 0xad, 0xb5, 0x1e, 0x01, 0xc1, 0xa3, 0x1e, 0x3f, 0xb4, 0x80, 0x18, 0x1b, 0xb2, 0xac, 0x36, 0x0b, 0x8c, 0x9c, 0x4a, 0x5e, 0x84, 0x84, 0x7a, 0x0e, 0x74, 0x84, 0x72, 0x0c, 0x16, 0xa8, 0x82, 0x68, 0xc6, 0xac, 0x72, 0x74, 0x86, 0x20, 0x32, 0x50, 0x86, 0x04, 0x2d };
	char test[55] = {0xca,0xa5, 0x61, 0xb4, 0x3c, 0x4e, 0x19, 0x50, 0xe2, 0xce, 0x33, 0xdb, 0x0e, 0x39, 0x99};
	HEADER header;

	header.len = 0xe;
	header.code = 0x77;
	header.RandKey = 0x5d;
	header.CheckSum = test[0];
	p->PutHeader(&header);

	//p->PutData(testData+1, 0xe);
	p->PutData(test + 1, 0xe);

	p->decode();

	//encode(p, 0x31);
	p->encode();
	p->GetData(temp, 54);

	for (int i = 1; i < 54; i++)
	{
		if (temp[i-1] != test[i])
		{
			system("pause");
		}
	}
	p->Clear();
	p->PutData(temp, 54);
	p->decode();
	//p->encode();
	//decode(p, 0x31);
	p->GetData(temp, 54);

	for (int i = 1; i < 54; i++)
	{
		if (temp[i-1] != testData[i])
		{
			system("pause");
		}
	}


	return 0;
}