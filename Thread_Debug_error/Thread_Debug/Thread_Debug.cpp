// Thread_Debug.cpp : �ܼ� ���� ���α׷��� ���� �������� �����մϴ�.
//

#include "stdafx.h"
#include "define.h"

////////////////////////////////////////////////////////
// ������ ��
////////////////////////////////////////////////////////

////////////////////////////////////////////////////////
// ���ӿ�û ���. 
//
// IOThread ���� �ֱ������� ����, 
// AcceptThread ���� �� ����Ʈ�� ��(SessionID ��)�� �̾Ƽ�  ���ο� Session �� �����.
// �̹� �����ϴ� SessionID �� ���ð�� ���� �ؾ� ��
////////////////////////////////////////////////////////
CRITICAL_SECTION	g_Accept_cs;
list<DWORD>			g_AcceptPacketList;

#define				LockAccept()	EnterCriticalSection(&g_Accept_cs)
#define 			UnlockAccept()	LeaveCriticalSection(&g_Accept_cs)

////////////////////////////////////////////////////////
// �׼� ��û ���.
//
// IOThread ���� �ֱ������� ����(SessionID),
// UpdateThread ������ �� ���� �̾Ƽ� �ش� �÷��̾��� Content + 1 �� �Ѵ�.
// �������� �ʴ� SessionID �� ������ �����ؾ� ��.
////////////////////////////////////////////////////////
CRITICAL_SECTION	g_Action_cs;
list<DWORD>			g_ActionPacketList;

#define				LockAction()	EnterCriticalSection(&g_Action_cs)
#define				UnlockAction()	LeaveCriticalSection(&g_Action_cs)

////////////////////////////////////////////////////////
// �������� ��û ���
//
// IOThread ���� �ֱ������� ����, 
// AcceptThread ���� �� ����Ʈ�� ���� �̾Ƽ� (SessionID ��) �ش� Session �� ���� ��Ų��.
////////////////////////////////////////////////////////
CRITICAL_SECTION	g_Disconnect_cs;
list<DWORD>			g_DisconnectPacketList;

#define				LockDisconnect()	EnterCriticalSection(&g_Disconnect_cs)
#define				UnlockDisconnect()	LeaveCriticalSection(&g_Disconnect_cs)






////////////////////////////////////////////////////////
// Session ���.
//
// ������ �Ϸ�� (Accept ó�� �Ϸ�)  st_SESSION �� ���� �����Ͽ�, SessionList �� �����͸� �ִ´�.
// �׸��� ������ ������ �� �ش� ������ ���� �Ѵ�.
////////////////////////////////////////////////////////
CRITICAL_SECTION		g_Session_cs;
list<st_SESSION *>		g_SessionList;

#define	LockSession()	EnterCriticalSection(&g_Session_cs)
#define UnlockSession()	LeaveCriticalSection(&g_Session_cs)

////////////////////////////////////////////////////////
// Player ���.
//
// Session �� ���� ��, ���� �ɶ� (Accept ó�� �Ϸ��)  st_PLAYER ��ü�� �Բ� �����Ǿ� ���⿡ ��� �ȴ�.
////////////////////////////////////////////////////////
CRITICAL_SECTION		g_Player_cs;
list<st_PLAYER *>		g_PlayerList;

#define LockPlayer()	EnterCriticalSection(&g_Player_cs)
#define UnlockPlayer()	LeaveCriticalSection(&g_Player_cs)



HANDLE	g_hExitThreadEvent;

HANDLE	g_hAcceptThreadEvent;
HANDLE	g_hUpdateThreadEvent;


WCHAR *g_szDebug;



void NewSession(DWORD dwSessionID)
{
	st_SESSION *pSession = new st_SESSION;
	pSession->SessionID = dwSessionID;
	
	LockSession();//a
	g_SessionList.push_back(pSession);
	UnlockSession();//a


	st_PLAYER *pPlayer = new st_PLAYER;
	pPlayer->SessionID = dwSessionID;

	//���� - ����
	//���� �ڵ�� sizeof(pPlayer->Content)*3�� ũ�⸸ŭ memset�� �ϴ� �ڵ�
	//�迭�� �̸��� �迭�� �����ּ�+�迭�� ���������� ����������
	//���� sizeof(pPlayer->Content)�� �迭�� ��Ҽ� * ����� ũ���̹Ƿ� *3�� ���� �ʿ䰡 ����
	//memset(pPlayer->Content, 0, sizeof(pPlayer->Content) * 3);
	memset(pPlayer->Content, 0, sizeof(pPlayer->Content));

	LockPlayer();//b
	g_PlayerList.push_back(pPlayer);
	UnlockPlayer();//b

}

void DeleteSession(DWORD dwSessionID)
{
	LockSession();//c

	list<st_SESSION *>::iterator SessionIter = g_SessionList.begin();
	for ( ; SessionIter != g_SessionList.end(); SessionIter++ )
	{
		if ( dwSessionID == (*SessionIter)->SessionID )
		{
			delete *SessionIter;
			g_SessionList.erase(SessionIter);
			break;
		}
	}
	UnlockSession();//c

	LockPlayer();//d
	list<st_PLAYER *>::iterator PlayerIter = g_PlayerList.begin();
	for ( ; PlayerIter != g_PlayerList.end(); PlayerIter++ )
	{
		if ( dwSessionID == (*PlayerIter)->SessionID )
		{
			delete *PlayerIter;
			g_PlayerList.erase(PlayerIter);
			//���� - ����
			//return���� ���������� �� ��� ������ LockPlayer()�� ���� playerList�� ���� Lock�� �ɰ�
			//Lock�� Ǯ���ʰ� �Լ��� ���������� �ȴ�.
			//return;
			break;
		}
	}
	UnlockPlayer();//d
}


bool FindSessionList(DWORD dwSessionID)
{
	bool ret = false;
	LockSession();//e

	list<st_SESSION *>::iterator SessionIter = g_SessionList.begin();


	for ( ; SessionIter != g_SessionList.end(); SessionIter++ )
	{
		if ( dwSessionID == (*SessionIter)->SessionID )
		{
			ret = true;
			break;
			//���� - ����
			//return���� ���������� LockSession�� ���� Lock�� Ǯ���� �ʰ� ����ǰԵȴ�.
			//���� �����ڵ忡�� FindSessionList�Լ��� ȣ���ϴ� ������� AcceptThread���̶� �̸� �������� �ʾƵ� �������� �۵��� �Ѵ�.
			//but, ���� ������ �ϰų� FindSessionList�� ȣ���� �ٸ� �����尡 ������ ��� RecursionCount��ŭ Unlock�� ������ؼ� ������ �߻���
			//return true;
		}
	}
	UnlockSession();//e

	//return false;
	return ret;
}



/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
// Accept Thread
/////////////////////////////////////////////////////////////////////////////////////
unsigned int WINAPI AcceptThread(LPVOID lpParam)
{
	HANDLE hEvent[2] = {g_hExitThreadEvent, g_hAcceptThreadEvent};
	DWORD dwError;
	DWORD dwSessionID;
	bool bLoop = true;
	bool bFindIt = false;

	wprintf(L"Accept Thread Start\n");
	srand(GetTickCount() + 1);

	while ( bLoop )
	{
		dwError = WaitForMultipleObjects(2, hEvent, FALSE, INFINITE);

		if ( dwError == WAIT_FAILED )
 		{
			wprintf(L"Accept Thread Event Error\n");
			wprintf(L"Accept Thread Event Error\n");
			wprintf(L"Accept Thread Event Error\n");
			wprintf(L"Accept Thread Event Error\n");
			break;
		}

		if ( dwError == WAIT_OBJECT_0 )
			break;


		//----------------------------------------------------------
		//----------------------------------------------------------
		// ���� ����ó�� 
		//----------------------------------------------------------
		//----------------------------------------------------------

		//----------------------------------------------------------
		// ���ӿ�û ó��
		//----------------------------------------------------------
		while ( !g_AcceptPacketList.empty() )
		{
			//���� - ����
			//g_AcceptPacketList.begin() ȣ�� ���� push_front�� �ٸ� �����忡�� �۵��ϰԵǸ�
			//���� pop_front�� ���� �� ������ �ʴ� ����� �߻��� �� �ִ�.
			//dwSessionID = *g_AcceptPacketList.begin();
			LockAccept();//f
			dwSessionID = *g_AcceptPacketList.begin();
			g_AcceptPacketList.pop_front();
			UnlockAccept();//f

			//----------------------------------------------------------
			// SessionList �� �̹� �����ϴ� SessionID ���� Ȯ��.  ���� ��츸 ���.
			//----------------------------------------------------------
			if ( !FindSessionList(dwSessionID) )
			{
				NewSession(dwSessionID);
				wprintf(L"AcceptThread - New Session[%d]\n", dwSessionID);
			}
		}

		//----------------------------------------------------------
		// �������� ó��
		//----------------------------------------------------------
		while ( !g_DisconnectPacketList.empty() )
		{
			LockDisconnect();//g
			dwSessionID = *g_DisconnectPacketList.begin();
			g_DisconnectPacketList.pop_front();
			UnlockDisconnect();//g

			//----------------------------------------------------------
			// SessionList �� �����ϴ� SessionID ���� Ȯ��.  �ִ� ��츸 ����
			//----------------------------------------------------------
			if ( FindSessionList(dwSessionID) )
			{
				DeleteSession(dwSessionID);
				wprintf(L"AcceptThread - Delete Session[%d]\n", dwSessionID);
			}
		}
	}

	wprintf(L"Accept Thread Exit\n");
	return 0;
}


/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
// IO Thread
/////////////////////////////////////////////////////////////////////////////////////
unsigned int WINAPI IOThread(LPVOID lpParam)
{
	DWORD dwError;
	bool bLoop = true;
	int iRand;
	DWORD dwSessionID;

	srand(GetTickCount() + 2);
	wprintf(L"IO Thread Start\n");

	while ( bLoop )
	{
		dwError = WaitForSingleObject(g_hExitThreadEvent, 10);
		if ( dwError != WAIT_TIMEOUT )
			break;

		//----------------------------------------------------------
		// ���� ����ó�� 
		//----------------------------------------------------------
		iRand = rand() % 3;
		dwSessionID = rand() % 5000;
		

		switch ( iRand )
		{
		case 0:			// Accept �߰�
			wsprintf(g_szDebug, L"# IOThread AcceptPacket Insert [%d] \n", dwSessionID);
			LockAccept();//h
			g_AcceptPacketList.push_back(dwSessionID);
			UnlockAccept();//h
			SetEvent(g_hAcceptThreadEvent);
			break;

		case 1:			// Disconnect �߰�
			wsprintf(g_szDebug, L"# IOThread DisconnetPacket Insert [%d] \n", dwSessionID);
			LockDisconnect();//i
			g_DisconnectPacketList.push_back(dwSessionID);
			UnlockDisconnect();//i
			SetEvent(g_hAcceptThreadEvent);
			break;

		case 2:			// Action �߰�
			wsprintf(g_szDebug, L"# IOThread ActionPacket Insert [%d] \n", dwSessionID);
			LockAction();//j
			g_ActionPacketList.push_back(dwSessionID);
			UnlockAction();//j
			SetEvent(g_hUpdateThreadEvent);
			break;
		}
		OutputDebugString(g_szDebug);
	}


	wprintf(L"IO Thread Exit\n");
	return 0;
}


/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
// Update Thread
/////////////////////////////////////////////////////////////////////////////////////
unsigned int WINAPI UpdateThread(LPVOID lpParam)
{
	HANDLE hEvent[2] = {g_hExitThreadEvent, g_hUpdateThreadEvent};
	DWORD dwError;
	DWORD dwSessionID;
	st_PLAYER *pPlayer;
	bool bLoop = true;

	srand(GetTickCount() + 3);

	wprintf(L"Update Thread Start\n");

	while ( bLoop )
	{
		dwError = WaitForMultipleObjects(2, hEvent, FALSE, INFINITE);

		if ( dwError == WAIT_FAILED )
 		{
			wprintf(L"Update Thread Event Error\n");
			wprintf(L"Update Thread Event Error\n");
			wprintf(L"Update Thread Event Error\n");
			wprintf(L"Update Thread Event Error\n");
			break;
		}

		if ( dwError == WAIT_OBJECT_0 )
			break;


		//----------------------------------------------------------
		// ���� ����ó�� 
		//----------------------------------------------------------
		//----------------------------------------------------------
		// �÷��̾� �׼� ó��
		//----------------------------------------------------------
		while ( !g_ActionPacketList.empty() )
		{
			LockAction();//k
			dwSessionID = *g_ActionPacketList.begin();
			g_ActionPacketList.pop_front();
			UnlockAction();//k
			//���� �ָ�?
			//ActionPacketList�� ���ؼ� Lock�� �Ŵ� LockAction�� ���� ���� ���� ���ο��� LockPlayer�� �ѵڿ� Action�� Ǯ������ϴ°� �ǹ�
			//g_ActionPacketList.pop_front();�� �� ���ĺ��ʹ� g_ActionPacketList�� ������ ���� �ʱ� ����
			//�� �̰�� action�� �ް��� player�� �����ϱ� ������ player�� delete�� ���� �ֱ���
			//�̰��� ������ �ǰų� ���� �̷����� �ʿ䰡 �ִ����� �ָ�
			

			//----------------------------------------------------------
			// PlayerList �� �̹� �����ϴ� SessionID ���� Ȯ��. �ִ� ��츸 �ش� �÷��̾� ã�Ƽ� + 1
			//----------------------------------------------------------
			LockPlayer();//l
			list<st_PLAYER *>::iterator PlayerIter = g_PlayerList.begin();
			for ( ; PlayerIter != g_PlayerList.end(); PlayerIter++ )
			{
				pPlayer = *PlayerIter;
				if ( dwSessionID == pPlayer->SessionID )
				{
					// ������ ������Ʈ - Content �迭���� + 1 �� ���

					//���� - ����
					//player�� Content�迭�� ũ��� 3�̴�.
					//�׷��� ������ ���� iCnt <= 3���� ��ȸ�ϴ� for���� ��� 0~3���� �� 4���� �迭�� �����ϰԵȴ�.
					//���� �Ҵ����� ���� ������ ���� �۾��� �ϰԵǾ� ������ �����.
					//for ( int iCnt = 0; iCnt <= 3; iCnt++ )
					for (int iCnt = 0; iCnt < 3; iCnt++)
					{
						pPlayer->Content[iCnt]++;
					}
					wprintf(L"UpdateThread - Session[%d] Content[%d] \n", dwSessionID, pPlayer->Content[0]);
					break;
				}
			}
			UnlockPlayer();//l
			//UnlockAction();//k

		}

	}

	wprintf(L"Update Thread Exit\n");
	return 0;
}




/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
// Initial
/////////////////////////////////////////////////////////////////////////////////////
void Initial()
{
	//���� - ����
	//g_szDebug�� ���� ���ڿ� �� ���� �� L"# IOThread DisconnetPacket Insert [%d] \n"�� ���̴� 39�̴�.(%d�� ���� ��ȯ���� ����)
	//������ %d�ڸ��� sessionID���� ���ԵǸ� sessionID���� �ִ� 4�ڸ� �����̹Ƿ� ���ڿ��� �ִ� 39+4�� 43�� ���̸� ������. 
	//40��ŭ �����Ҵ��� �ϰԵǸ� wsprintf�Լ� ���� ������ �޸𸮸� �Ѿ ħ���ϰ� �ȴ�.
	//���� �ּ� 43�ڸ���ŭ �Ҵ��� ������Ѵ�( �����Ӱ� 50����)
	//g_szDebug = new WCHAR[40];
	g_szDebug = new WCHAR[50];

	//------------------------------------------------
	// ������ �����带 ���� �̺�Ʈ
	//------------------------------------------------
	g_hAcceptThreadEvent	= CreateEvent(NULL, FALSE, FALSE, NULL);
	g_hUpdateThreadEvent	= CreateEvent(NULL, FALSE, FALSE, NULL);

	//------------------------------------------------
	// ��� �����带 ���� ��ų �̺�Ʈ
	//------------------------------------------------

	//���� - ����
	//��� �����带 �����Ű�� Event�̹Ƿ� exitThreadEvent�� ��� �����尡 �����ؾ���
	//manual reset�� ������Ѵ�
	//g_hExitThreadEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	g_hExitThreadEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	InitializeCriticalSection(&g_Accept_cs);
	InitializeCriticalSection(&g_Action_cs);
	InitializeCriticalSection(&g_Disconnect_cs);
	InitializeCriticalSection(&g_Player_cs);
	InitializeCriticalSection(&g_Session_cs);

}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
// Release
/////////////////////////////////////////////////////////////////////////////////////
void Release()
{
	g_AcceptPacketList.clear();
	g_ActionPacketList.clear();
	g_DisconnectPacketList.clear();


	list<st_SESSION *>::iterator SessionIter = g_SessionList.begin();
	while ( SessionIter != g_SessionList.end() )
	{
		delete *SessionIter;
		//���� - ����
		//erase�� �ϰ��� SessionIter�� ������ ��ġ�� ����Ű�� iterator�� ������
		//���� erase���� ������ �������� �ʿ䰡 ����
		//g_SessionList.erase(SessionIter);
		g_SessionList.erase(SessionIter++);
	}

	list<st_PLAYER *>::iterator PlayerIter = g_PlayerList.begin();
	while ( PlayerIter != g_PlayerList.end() )
	{
		delete *PlayerIter;
		//���� - ����
		//erase�� �ϰ��� PlayerIter�� ������ ��ġ�� ����Ű�� iterator�� ������
		//���� erase���� ������ �������� �ʿ䰡 ����
		//g_PlayerList.erase(PlayerIter);
		g_PlayerList.erase(PlayerIter++);
	}

	delete[] g_szDebug;

	DeleteCriticalSection(&g_Accept_cs);
	DeleteCriticalSection(&g_Action_cs);
	DeleteCriticalSection(&g_Disconnect_cs);
	DeleteCriticalSection(&g_Player_cs);
	DeleteCriticalSection(&g_Session_cs);


}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
// Main
/////////////////////////////////////////////////////////////////////////////////////
int _tmain(int argc, _TCHAR* argv[])
{
	HANDLE hIOThread;			// ���ӿ�û, �����û, �׼ǿ�û �߻�.  (IO �ùķ��̼�)
	HANDLE hAcceptThread;		// ���ӿ�û, ���⿡ ���� ó��	
	HANDLE hUpdateThread;		// �׼ǿ�û ó��.

	DWORD dwThreadID;

	Initial();



	//------------------------------------------------
	// ������ ����.
	//------------------------------------------------
	hAcceptThread	= (HANDLE)_beginthreadex(NULL, 0, AcceptThread, (LPVOID)0, 0, (unsigned int *)&dwThreadID);
	hIOThread		= (HANDLE)_beginthreadex(NULL, 0, IOThread,	(LPVOID)0, 0, (unsigned int *)&dwThreadID);
	hUpdateThread	= (HANDLE)_beginthreadex(NULL, 0, UpdateThread, (LPVOID)0, 0, (unsigned int *)&dwThreadID);


	WCHAR ControlKey;

	//------------------------------------------------
	// ���� ��Ʈ��...
	//------------------------------------------------
	while ( 1 )
	{	
		ControlKey = _getwch();
		if ( ControlKey == L'q' || ControlKey == L'Q' )
		{
			//------------------------------------------------
			// ����ó��
			//------------------------------------------------
			SetEvent(g_hExitThreadEvent);
			break;
		}
	}


	//------------------------------------------------
	// ������ ���� ���
	//------------------------------------------------
	HANDLE hThread[3] = {hAcceptThread, hIOThread, hUpdateThread};

	WaitForMultipleObjects(dfTHREAD_NUM, hThread, TRUE, INFINITE);


	Release();


	//------------------------------------------------
	// ������ �ڵ�  ������ �������� Ȯ��.
	//------------------------------------------------
	DWORD ExitCode;

	wprintf(L"\n\n--- THREAD CHECK LOG -----------------------------\n\n");

	GetExitCodeThread(hAcceptThread, &ExitCode);
	if ( ExitCode != 0 )
		wprintf(L"error - Accept Thread not exit\n");

	GetExitCodeThread(hIOThread, &ExitCode);
	if ( ExitCode != 0 )
		wprintf(L"error - IO Thread not exit\n");
	
	GetExitCodeThread(hUpdateThread, &ExitCode);
	if ( ExitCode != 0 )
		wprintf(L"error - Update Thread not exit\n");
	//system("pause");
	return 0;
}

