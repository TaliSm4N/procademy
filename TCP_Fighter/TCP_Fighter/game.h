#include "ScreenDib.h"
#include <Windows.h>

//ȭ�� �̵� ����
#define dfRANGE_MOVE_TOP	50
#define dfRANGE_MOVE_LEFT	10
#define dfRANGE_MOVE_RIGHT	630
#define dfRANGE_MOVE_BOTTOM	470

//������
#define dfDELAY_STAND	5
#define dfDELAY_MOVE	4
#define dfDELAY_ATTACK1	3
#define dfDELAY_ATTACK2	4
#define dfDELAY_ATTACK3	4
#define dfDELAY_EFFECT	3

ScreenDib g_ScreenDib(640, 480, 32);
HWND g_hWnd;

void Update();
void initGame(HWND hWnd);