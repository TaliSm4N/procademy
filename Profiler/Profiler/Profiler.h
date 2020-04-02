#pragma once

#define PRO_BEGIN(TagName) ProfileBegin(TagName)
#define PRO_END(TagName) ProfileEnd(TagName)


typedef struct
{
	long lFlag; // 프로파일의 사용 여부. (배열시에만)
	WCHAR szName[64]; // 프로파일 샘플 이름.

	LARGE_INTEGER lStartTime; // 프로파일 샘플 실행 시간.

	__int64 iTotalTime; // 전체 사용시간 카운터 Time. (출력시 호출회수로 나누어 평균 구함)
	__int64 iMin[2]; // 최소 사용시간 카운터 Time. (초단위로 계산하여 저장 / [0] 가장최소 [1])
	__int64 iMax[2]; // 최대 사용시간 카운터 Time. (초단위로 계산하여 저장 / [0] 가장최대 [1])

	__int64 iCall; // 누적 호출 횟수.

} PROFILE_SAMPLE;



/////////////////////////////////////////////////////////////////////////////
// 하나의 함수 Profiling 시작, 끝 함수.
//
// Parameters: (char *)Profiling이름.
// Return: 없음.
/////////////////////////////////////////////////////////////////////////////
void ProfileBegin(const WCHAR *szName);
void ProfileEnd(const WCHAR *szName);

/////////////////////////////////////////////////////////////////////////////
// Profiling 된 데이타를 Text 파일로 출력한다.
//
// Parameters: (char *)출력될 파일 이름.
// Return: 없음.
/////////////////////////////////////////////////////////////////////////////
void ProfileDataOutText(const WCHAR *szFileName);

/////////////////////////////////////////////////////////////////////////////
// 프로파일링 된 데이터를 모두 초기화 한다.
//
// Parameters: 없음.
// Return: 없음.
/////////////////////////////////////////////////////////////////////////////
void ProfileReset(void);
