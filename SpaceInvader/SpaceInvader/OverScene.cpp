#define _CRT_SECURE_NO_WARNINGS
#include "BaseObject.h"
#include "OverScene.h"
#include "UIObject.h"
#include "SceneManager.h"
#include "score.h"
#include <iostream>
#include <Windows.h>

OverScene::OverScene()
{
	char str[61];
	sprintf(str, "SCORE   %07d", score);
	ObjectManager.push_back(new FrameObject());
	ObjectManager.push_back(new StringObject(22, 15, "G A M E  O V E R"));
	ObjectManager.push_back(new StringObject(23, 17, str));
}

void OverScene::Update()
{
	if (GetAsyncKeyState(VK_SPACE))
	{
		score = 0;
		SceneManager::GetInstance()->LoadScene(SceneFlag::TITLE);
	}

	for (auto iter = ObjectManager.begin(); iter != ObjectManager.end();iter++)
	{
		(*iter)->Action();
	}

	for (auto iter = ObjectManager.begin(); iter != ObjectManager.end();iter++)
	{
		(*iter)->Draw();
	}
}