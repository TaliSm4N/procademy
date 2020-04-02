#pragma once

#define PRO_BEGIN(TagName) ProfileBegin(TagName)
#define PRO_END(TagName) ProfileEnd(TagName)


typedef struct
{
	long lFlag; // ���������� ��� ����. (�迭�ÿ���)
	WCHAR szName[64]; // �������� ���� �̸�.

	LARGE_INTEGER lStartTime; // �������� ���� ���� �ð�.

	__int64 iTotalTime; // ��ü ���ð� ī���� Time. (��½� ȣ��ȸ���� ������ ��� ����)
	__int64 iMin[2]; // �ּ� ���ð� ī���� Time. (�ʴ����� ����Ͽ� ���� / [0] �����ּ� [1])
	__int64 iMax[2]; // �ִ� ���ð� ī���� Time. (�ʴ����� ����Ͽ� ���� / [0] �����ִ� [1])

	__int64 iCall; // ���� ȣ�� Ƚ��.

} PROFILE_SAMPLE;



/////////////////////////////////////////////////////////////////////////////
// �ϳ��� �Լ� Profiling ����, �� �Լ�.
//
// Parameters: (char *)Profiling�̸�.
// Return: ����.
/////////////////////////////////////////////////////////////////////////////
void ProfileBegin(const WCHAR *szName);
void ProfileEnd(const WCHAR *szName);

/////////////////////////////////////////////////////////////////////////////
// Profiling �� ����Ÿ�� Text ���Ϸ� ����Ѵ�.
//
// Parameters: (char *)��µ� ���� �̸�.
// Return: ����.
/////////////////////////////////////////////////////////////////////////////
void ProfileDataOutText(const WCHAR *szFileName);

/////////////////////////////////////////////////////////////////////////////
// �������ϸ� �� �����͸� ��� �ʱ�ȭ �Ѵ�.
//
// Parameters: ����.
// Return: ����.
/////////////////////////////////////////////////////////////////////////////
void ProfileReset(void);
