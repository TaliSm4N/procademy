#ifndef __DEFINE__
#define __DEFINE__


struct st_SESSION
{
	int SessionID;
};


struct st_PLAYER
{
	int SessionID;
	int Content[3];
};

//문제 - 수정
//thread의 수는 3개(main Thread제외)
//따라서 4-> 3으로 수정
//이를 수정하지 않으면 WaitForMultipleObjects가 비정상 작동
//#define dfTHREAD_NUM	4
#define dfTHREAD_NUM	3
#endif