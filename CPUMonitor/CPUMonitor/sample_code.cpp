#define _CRT_SECURE_NO_WARNINGS

#ifdef WIN32
#  define CRTDBG_MAP_ALLOC
#  include <crtdbg.h>
#endif

#include <stdio.h>
#include <time.h>
#include <pdh.h>
#include <windows.h>

#ifndef __cplusplus
#  define bool int
#  define true 1
#  define false 0
#endif

#pragma comment(lib, "pdh.lib")

typedef enum ETH_QUERY_TYPE
{
	EQT_CURRENT_BANDWIDTH = 0,
	EQT_BYTES_RECV_PER_SEC = 1,
	EQT_BYTES_SENT_PER_SEC = 2,
	EQT_PACKET_RECV_PER_SEC = 3,
	EQT_PACKET_SENT_PER_SEC = 4,
	EQT_COUNT = 5
} ETH_QUERY_TYPE;

typedef struct Eth
{
	char *m_pszName;
	HQUERY m_hQuery;
	HCOUNTER m_hCounter[EQT_COUNT];
	unsigned long m_ulBandwidth;
	unsigned long m_ulBytesRecvPerSec;
	unsigned long m_ulBytesSentPerSec;
	unsigned long m_ulPacketRecvPerSec;
	unsigned long m_ulPacketSentPerSec;
} Eth;

typedef struct EthList
{
	int m_nCount;
	Eth *m_pEth;
} EthList;

bool OpenEthList(EthList *ethList)
{
	int i = 0;
	bool bErr = false;
	char *szCur = NULL;
	char *szBuff = NULL;
	char *szNet = NULL;
	DWORD dwBuffSize = 0, dwNetSize = 0;
	char szCounter[256];

	if (ethList == NULL)
		return false;

	memset(ethList, 0x00, sizeof(EthList));

	// calculate buffer size
	PdhEnumObjectItemsA(NULL, NULL, "Network Interface", szBuff, &dwBuffSize, szNet, &dwNetSize, PERF_DETAIL_WIZARD, 0);

	if ((szBuff = (char *)malloc(sizeof(char) * dwBuffSize)) == NULL)
	{
		return false;
	}

	if ((szNet = (char *)malloc(sizeof(char) * dwNetSize)) == NULL)
	{
		free(szBuff);
		return false;
	}

	if (PdhEnumObjectItemsA(NULL, NULL, "Network Interface", szBuff, &dwBuffSize, szNet, &dwNetSize, PERF_DETAIL_WIZARD, 0) != ERROR_SUCCESS)
	{
		free(szBuff);
		free(szNet);
		return false;
	}

	for (szCur = szNet, i = 0; *szCur != 0; szCur += lstrlenA(szCur) + 1, i++)
	{
		// Just count how many network interface is exist.
	}

	if (i <= 0)
	{
		free(szBuff);
		free(szNet);
		return false;
	}

	ethList->m_nCount = i;
	ethList->m_pEth = (Eth *)malloc(sizeof(Eth) * i);
	memset(ethList->m_pEth, 0x00, sizeof(Eth) * i);

	for (szCur = szNet, i = 0; *szCur != 0; szCur += lstrlenA(szCur) + 1, i++)
	{
		Eth *eth = &ethList->m_pEth[i];
		eth->m_pszName = _strdup(szCur);
		printf("test --- %s\n",szCur);
		if (PdhOpenQuery(NULL, 0, &eth->m_hQuery))
		{
			bErr = true;
			break;
		}

		sprintf(szCounter, "\\Network Interface(%s)\\Current Bandwidth", eth->m_pszName);
		if (PdhAddCounterA(eth->m_hQuery, szCounter, 0, &eth->m_hCounter[EQT_CURRENT_BANDWIDTH]))
		{
			bErr = true;
			break;
		}

		sprintf(szCounter, "\\Network Interface(%s)\\Bytes Received/sec", eth->m_pszName);
		if (PdhAddCounterA(eth->m_hQuery, szCounter, 0, &eth->m_hCounter[EQT_BYTES_RECV_PER_SEC]))
		{
			bErr = true;
			break;
		}

		sprintf(szCounter, "\\Network Interface(%s)\\Bytes Sent/sec", eth->m_pszName);
		if (PdhAddCounterA(eth->m_hQuery, szCounter, 0, &eth->m_hCounter[EQT_BYTES_SENT_PER_SEC]))
		{
			bErr = true;
			break;
		}

		sprintf(szCounter, "\\Network Interface(%s)\\Packets Received/sec", eth->m_pszName);
		if (PdhAddCounterA(eth->m_hQuery, szCounter, 0, &eth->m_hCounter[EQT_PACKET_RECV_PER_SEC]))
		{
			bErr = true;
			break;
		}

		sprintf(szCounter, "\\Network Interface(%s)\\Packets Sent/sec", eth->m_pszName);
		if (PdhAddCounterA(eth->m_hQuery, szCounter, 0, &eth->m_hCounter[EQT_PACKET_SENT_PER_SEC]))
		{
			bErr = true;
			break;
		}
	}

	if (szBuff != NULL)
		free(szBuff);

	if (szNet != NULL)
		free(szNet);

	if (bErr)
		return false;

	return true;
}

bool QueryEthUsage(Eth *eth)
{
	HCOUNTER hCounter = NULL;

	PDH_FMT_COUNTERVALUE v;

	if (eth == NULL)
		return false;

	if (PdhCollectQueryData(eth->m_hQuery))
	{
		return false;
	}

	v.longValue = 0;
	PdhGetFormattedCounterValue(eth->m_hCounter[EQT_CURRENT_BANDWIDTH], PDH_FMT_LONG, 0, &v);
	eth->m_ulBandwidth = v.longValue;

	v.longValue = 0;
	PdhGetFormattedCounterValue(eth->m_hCounter[EQT_BYTES_RECV_PER_SEC], PDH_FMT_LONG, 0, &v);
	eth->m_ulBytesRecvPerSec = v.longValue;

	v.longValue = 0;
	PdhGetFormattedCounterValue(eth->m_hCounter[EQT_BYTES_SENT_PER_SEC], PDH_FMT_LONG, 0, &v);
	eth->m_ulBytesSentPerSec = v.longValue;

	v.longValue = 0;
	PdhGetFormattedCounterValue(eth->m_hCounter[EQT_PACKET_RECV_PER_SEC], PDH_FMT_LONG, 0, &v);
	eth->m_ulPacketRecvPerSec = v.longValue;

	v.longValue = 0;
	PdhGetFormattedCounterValue(eth->m_hCounter[EQT_PACKET_SENT_PER_SEC], PDH_FMT_LONG, 0, &v);
	eth->m_ulPacketSentPerSec = v.longValue;

	return true;
}

void CloseEthList(EthList *ethList)
{
	if (ethList != NULL)
	{
		int i = 0, j = 0;
		for (i = 0; i < ethList->m_nCount; i++)
		{
			Eth *eth = &ethList->m_pEth[i];
			free(eth->m_pszName);

			for (j = 0; j < EQT_COUNT; j++)
			{
				if (eth->m_hCounter[j] != NULL)
				{
					PdhRemoveCounter(eth->m_hCounter[j]);
					eth->m_hCounter[j] = NULL;
				}
			}

			PdhCloseQuery(eth->m_hQuery);
			eth->m_hQuery = NULL;
		}

		free(ethList->m_pEth);
		memset(ethList, 0x00, sizeof(EthList));
	}
}

int main(int argc, char* argv[])
{
	int i = 0;
	time_t tLast = time(0) + 20;
	EthList ethList;

	//_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF | _CRTDBG_DELAY_FREE_MEM_DF);

	if (OpenEthList(&ethList) <= 0)
	{
		printf("fail to load ethernet list\n");
	}

	while (time(0) < tLast)
	{
		for (i = 0; i < ethList.m_nCount; i++)
		{
			Eth *e = &ethList.m_pEth[i];

			if (!QueryEthUsage(e))
				break;

			printf("%s\n", e->m_pszName);
			printf("bandwidth %ld\n", e->m_ulBandwidth);
			printf("bytes recv/sec %ld\n", e->m_ulBytesRecvPerSec);
			printf("bytes sent/sec %ld\n", e->m_ulBytesSentPerSec);
			printf("packet recv/sec %ld\n", e->m_ulPacketRecvPerSec);
			printf("packet sent/sec %ld\n", e->m_ulPacketSentPerSec);
		}

		Sleep(1000);
		system("cls");
	}

	CloseEthList(&ethList);
	//_CrtDumpMemoryLeaks();
	return 0;
}