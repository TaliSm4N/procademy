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

//���� - ����
//thread�� ���� 3��(main Thread����)
//���� 4-> 3���� ����
//�̸� �������� ������ WaitForMultipleObjects�� ������ �۵�
//#define dfTHREAD_NUM	4
#define dfTHREAD_NUM	3
#endif