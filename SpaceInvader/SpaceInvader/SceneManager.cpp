#include "SceneManager.h"
#include "TitleScene.h"
#include "GameScene.h"
#include "OverScene.h"

SceneManager *SceneManager::GetInstance()
{
	if (_sManager == nullptr)
	{
		_sManager = new SceneManager();
	}

	return _sManager;
}

void SceneManager::run()
{
	now->Update();

	if (next != nullptr)
	{
		delete now;
		now = next;
		next = nullptr;
	}
}

SceneManager::SceneManager():next(nullptr)
{
	now = new TitleScene();
}

void SceneManager::LoadScene(SceneFlag f)
{
	switch (f)
	{
	case TITLE:
		next = new TitleScene();
		break;
	case GAME:
		next = new GameScene();
		break;
	case OVER:
		next = new OverScene();
		break;
	default:
		break;
	}
}

SceneManager *SceneManager::_sManager = nullptr;