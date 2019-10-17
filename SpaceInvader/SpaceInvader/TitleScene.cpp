#include "BaseObject.h"
#include "TitleScene.h"
#include "Screen.h"
#include "UIObject.h"
#include "SceneManager.h"
#include <Windows.h>


TitleScene::TitleScene()
{
	BaseObject *temp = new FrameObject();
	ObjectManager.push_back(temp);

	temp = new StringObject(22, 15, "- SHOOTING  GAME -");
	ObjectManager.push_back(temp);

	temp = new StringObject(22, 16, "- Press Space Key ");
	ObjectManager.push_back(temp);
}

void TitleScene::Update()
{
	if (GetAsyncKeyState(VK_SPACE))
	{
		SceneManager::GetInstance()->LoadScene(SceneFlag::GAME);
	}
	for (auto iter = ObjectManager.begin(); iter != ObjectManager.end(); iter++)
	{
		(*iter)->Action();
	}
	for (auto iter = ObjectManager.begin(); iter != ObjectManager.end(); iter++)
	{
		(*iter)->Draw();
	}
}