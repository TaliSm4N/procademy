#include "Buffer.h"
#include "Console.h"
#include <stdio.h>
#include <memory.h>

//--------------------------------------------------------------------
// 버퍼의 내용을 화면으로 찍어주는 함수.
//
// 적군,아군,총알 등을 szScreenBuffer 에 넣어주고, 
// 1 프레임이 끝나는 마지막에 본 함수를 호출하여 버퍼 -> 화면 으로 그린다.
//--------------------------------------------------------------------

char szScreenBuffer[dfSCREEN_HEIGHT][dfSCREEN_WIDTH];

void Buffer_Flip(void)
{
	for (int i = 0; i < dfSCREEN_HEIGHT; i++)
	{
		cs_MoveCursor(0, i);
		printf("%s", szScreenBuffer[i]);

	}
}


//--------------------------------------------------------------------
// 화면 버퍼를 지워주는 함수
//
// 매 프레임 그림을 그리기 직전에 버퍼를 지워 준다. 
// 안그러면 이전 프레임의 잔상이 남으니까
//--------------------------------------------------------------------
void Buffer_Clear(void)
{
	memset(szScreenBuffer, ' ', dfSCREEN_HEIGHT*dfSCREEN_WIDTH);

	for (int i = 0; i < dfSCREEN_HEIGHT; i++)
	{
		szScreenBuffer[i][dfSCREEN_WIDTH - 1] = '\0';
	}
}

//--------------------------------------------------------------------
// 버퍼의 특정 위치에 원하는 문자를 출력.
//
// 입력 받은 X,Y 좌표에 아스키코드 하나를 출력한다. (버퍼에 그림)
//--------------------------------------------------------------------
void Sprite_Draw(int iX, int iY, char chSprite)
{
	szScreenBuffer[iY][iX] = chSprite;
}