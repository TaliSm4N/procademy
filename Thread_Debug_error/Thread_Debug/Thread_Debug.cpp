// Thread_Debug.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include "define.h"

////////////////////////////////////////////////////////
// 컨텐츠 부
////////////////////////////////////////////////////////

////////////////////////////////////////////////////////
// 접속요청 목록. 
//
// IOThread 에서 주기적으로 삽입, 
// AcceptThread 에서 이 리스트의 값(SessionID 값)을 뽑아서  새로운 Session 을 만든다.
// 이미 존재하는 SessionID 가 나올경우 무시 해야 함
////////////////////////////////////////////////////////
CRITICAL_SECTION	g_Accept_cs;
list<DWORD>			g_AcceptPacketList;

#define				LockAccept()	EnterCriticalSection(&g_Accept_cs)
#define 			UnlockAccept()	LeaveCriticalSection(&g_Accept_cs)

////////////////////////////////////////////////////////
// 액션 요청 목록.
//
// IOThread 에서 주기적으로 삽입(SessionID),
// UpdateThread 에서는 이 값을 뽑아서 해당 플레이어의 Content + 1 을 한다.
// 존재하지 않는 SessionID 가 나오면 무시해야 함.
////////////////////////////////////////////////////////
CRITICAL_SECTION	g_Action_cs;
list<DWORD>			g_ActionPacketList;

#define				LockAction()	EnterCriticalSection(&g_Action_cs)
#define				UnlockAction()	LeaveCriticalSection(&g_Action_cs)

////////////////////////////////////////////////////////
// 접속종료 요청 목록
//
// IOThread 에서 주기적으로 삽입, 
// AcceptThread 에서 이 리스트의 값을 뽑아서 (SessionID 값) 해당 Session 을 종료 시킨다.
////////////////////////////////////////////////////////
CRITICAL_SECTION	g_Disconnect_cs;
list<DWORD>			g_DisconnectPacketList;

#define				LockDisconnect()	EnterCriticalSection(&g_Disconnect_cs)
#define				UnlockDisconnect()	LeaveCriticalSection(&g_Disconnect_cs)






////////////////////////////////////////////////////////
// Session 목록.
//
// 접속이 완료시 (Accept 처리 완료)  st_SESSION 를 동적 생성하여, SessionList 에 포인터를 넣는다.
// 그리고 접속이 끊어질 시 해당 세션을 삭제 한다.
////////////////////////////////////////////////////////
CRITICAL_SECTION		g_Session_cs;
list<st_SESSION *>		g_SessionList;

#define	LockSession()	EnterCriticalSection(&g_Session_cs)
#define UnlockSession()	LeaveCriticalSection(&g_Session_cs)

////////////////////////////////////////////////////////
// Player 목록.
//
// Session 이 생성 후, 생성 될때 (Accept 처리 완료시)  st_PLAYER 객체도 함께 생성되어 여기에 등록 된다.
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

	//문제 - 수정
	//기존 코드는 sizeof(pPlayer->Content)*3의 크기만큼 memset을 하는 코드
	//배열의 이름은 배열의 시작주소+배열의 길이정보를 가지고있음
	//따라서 sizeof(pPlayer->Content)은 배열의 요소수 * 요소의 크기이므로 *3을 해줄 필요가 없음
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
			//문제 - 수정
			//return으로 빠져나가게 될 경우 이전에 LockPlayer()를 통해 playerList에 대한 Lock을 걸고
			//Lock을 풀지않고 함수를 빠져나가게 된다.
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
			//문제 - 수정
			//return으로 빠져나가면 LockSession에 의한 Lock이 풀리지 않고 진행되게된다.
			//물론 현재코드에서 FindSessionList함수를 호출하는 쓰레드는 AcceptThread뿐이라서 이를 수정하지 않아도 문제없이 작동은 한다.
			//but, 이후 수정을 하거나 FindSessionList를 호출할 다른 쓰레드가 생겼을 경우 RecursionCount만큼 Unlock을 해줘야해서 문제가 발생함
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
		// 정상 로직처리 
		//----------------------------------------------------------
		//----------------------------------------------------------

		//----------------------------------------------------------
		// 접속요청 처리
		//----------------------------------------------------------
		while ( !g_AcceptPacketList.empty() )
		{
			//문제 - 수정
			//g_AcceptPacketList.begin() 호출 이후 push_front가 다른 쓰레드에서 작동하게되면
			//이후 pop_front를 했을 때 원하지 않는 결과가 발생할 수 있다.
			//dwSessionID = *g_AcceptPacketList.begin();
			LockAccept();//f
			dwSessionID = *g_AcceptPacketList.begin();
			g_AcceptPacketList.pop_front();
			UnlockAccept();//f

			//----------------------------------------------------------
			// SessionList 에 이미 존재하는 SessionID 인지 확인.  없는 경우만 등록.
			//----------------------------------------------------------
			if ( !FindSessionList(dwSessionID) )
			{
				NewSession(dwSessionID);
				wprintf(L"AcceptThread - New Session[%d]\n", dwSessionID);
			}
		}

		//----------------------------------------------------------
		// 접속해제 처리
		//----------------------------------------------------------
		while ( !g_DisconnectPacketList.empty() )
		{
			LockDisconnect();//g
			dwSessionID = *g_DisconnectPacketList.begin();
			g_DisconnectPacketList.pop_front();
			UnlockDisconnect();//g

			//----------------------------------------------------------
			// SessionList 에 존재하는 SessionID 인지 확인.  있는 경우만 삭제
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
		// 정상 로직처리 
		//----------------------------------------------------------
		iRand = rand() % 3;
		dwSessionID = rand() % 5000;
		

		switch ( iRand )
		{
		case 0:			// Accept 추가
			wsprintf(g_szDebug, L"# IOThread AcceptPacket Insert [%d] \n", dwSessionID);
			LockAccept();//h
			g_AcceptPacketList.push_back(dwSessionID);
			UnlockAccept();//h
			SetEvent(g_hAcceptThreadEvent);
			break;

		case 1:			// Disconnect 추가
			wsprintf(g_szDebug, L"# IOThread DisconnetPacket Insert [%d] \n", dwSessionID);
			LockDisconnect();//i
			g_DisconnectPacketList.push_back(dwSessionID);
			UnlockDisconnect();//i
			SetEvent(g_hAcceptThreadEvent);
			break;

		case 2:			// Action 추가
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
		// 정상 로직처리 
		//----------------------------------------------------------
		//----------------------------------------------------------
		// 플레이어 액션 처리
		//----------------------------------------------------------
		while ( !g_ActionPacketList.empty() )
		{
			LockAction();//k
			dwSessionID = *g_ActionPacketList.begin();
			g_ActionPacketList.pop_front();
			UnlockAction();//k
			//문제 애매?
			//ActionPacketList에 대해서 Lock을 거는 LockAction의 수행 이후 굳이 내부에서 LockPlayer를 한뒤에 Action을 풀어줘야하는가 의문
			//g_ActionPacketList.pop_front();를 한 이후부터는 g_ActionPacketList에 접근을 하지 않기 때문
			//단 이경우 action을 받고나서 player가 수행하기 이전에 player가 delete될 수도 있긴함
			//이것이 문제가 되거나 굳이 이렇게할 필요가 있는지는 애매
			

			//----------------------------------------------------------
			// PlayerList 에 이미 존재하는 SessionID 인지 확인. 있는 경우만 해당 플레이어 찾아서 + 1
			//----------------------------------------------------------
			LockPlayer();//l
			list<st_PLAYER *>::iterator PlayerIter = g_PlayerList.begin();
			for ( ; PlayerIter != g_PlayerList.end(); PlayerIter++ )
			{
				pPlayer = *PlayerIter;
				if ( dwSessionID == pPlayer->SessionID )
				{
					// 컨텐츠 업데이트 - Content 배열마다 + 1 후 출력

					//문제 - 수정
					//player의 Content배열의 크기는 3이다.
					//그런데 다음과 같이 iCnt <= 3까지 순회하는 for문의 경우 0~3까지 즉 4개의 배열에 동작하게된다.
					//따라서 할당하지 않은 공간에 대한 작업을 하게되어 문제가 생긴다.
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
	//문제 - 수정
	//g_szDebug에 들어가는 문자열 중 가장 긴 L"# IOThread DisconnetPacket Insert [%d] \n"의 길이는 39이다.(%d와 같은 변환문자 제외)
	//실제로 %d자리에 sessionID값이 들어가게되면 sessionID값은 최대 4자리 숫자이므로 문자열은 최대 39+4의 43의 길이를 가진다. 
	//40만큼 동적할당을 하게되면 wsprintf함수 사용시 지정한 메모리를 넘어서 침범하게 된다.
	//따라서 최소 43자리만큼 할당을 해줘야한다( 여유롭게 50잡음)
	//g_szDebug = new WCHAR[40];
	g_szDebug = new WCHAR[50];

	//------------------------------------------------
	// 각각의 스레드를 깨울 이벤트
	//------------------------------------------------
	g_hAcceptThreadEvent	= CreateEvent(NULL, FALSE, FALSE, NULL);
	g_hUpdateThreadEvent	= CreateEvent(NULL, FALSE, FALSE, NULL);

	//------------------------------------------------
	// 모든 스레드를 종료 시킬 이벤트
	//------------------------------------------------

	//문제 - 수정
	//모든 쓰레드를 종료시키는 Event이므로 exitThreadEvent로 모든 쓰레드가 반응해야함
	//manual reset을 해줘야한다
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
		//문제 - 수정
		//erase를 하고나서 SessionIter는 기존의 위치를 가리키는 iterator로 유지됨
		//따라서 erase해줄 때마다 움직여줄 필요가 있음
		//g_SessionList.erase(SessionIter);
		g_SessionList.erase(SessionIter++);
	}

	list<st_PLAYER *>::iterator PlayerIter = g_PlayerList.begin();
	while ( PlayerIter != g_PlayerList.end() )
	{
		delete *PlayerIter;
		//문제 - 수정
		//erase를 하고나서 PlayerIter는 기존의 위치를 가리키는 iterator로 유지됨
		//따라서 erase해줄 때마다 움직여줄 필요가 있음
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
	HANDLE hIOThread;			// 접속요청, 끊기요청, 액션요청 발생.  (IO 시뮬레이션)
	HANDLE hAcceptThread;		// 접속요청, 끊기에 대한 처리	
	HANDLE hUpdateThread;		// 액션요청 처리.

	DWORD dwThreadID;

	Initial();



	//------------------------------------------------
	// 스레드 생성.
	//------------------------------------------------
	hAcceptThread	= (HANDLE)_beginthreadex(NULL, 0, AcceptThread, (LPVOID)0, 0, (unsigned int *)&dwThreadID);
	hIOThread		= (HANDLE)_beginthreadex(NULL, 0, IOThread,	(LPVOID)0, 0, (unsigned int *)&dwThreadID);
	hUpdateThread	= (HANDLE)_beginthreadex(NULL, 0, UpdateThread, (LPVOID)0, 0, (unsigned int *)&dwThreadID);


	WCHAR ControlKey;

	//------------------------------------------------
	// 종료 컨트롤...
	//------------------------------------------------
	while ( 1 )
	{	
		ControlKey = _getwch();
		if ( ControlKey == L'q' || ControlKey == L'Q' )
		{
			//------------------------------------------------
			// 종료처리
			//------------------------------------------------
			SetEvent(g_hExitThreadEvent);
			break;
		}
	}


	//------------------------------------------------
	// 스레드 종료 대기
	//------------------------------------------------
	HANDLE hThread[3] = {hAcceptThread, hIOThread, hUpdateThread};

	WaitForMultipleObjects(dfTHREAD_NUM, hThread, TRUE, INFINITE);


	Release();


	//------------------------------------------------
	// 디버깅용 코드  스레드 정상종료 확인.
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

