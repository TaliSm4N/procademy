#include "BaseObject.h"
#include "GameScene.h"
#include "UIObject.h"
#include "PlayerObject.h"
#include "EnemyObject.h"
#include "SceneManager.h"
#include "score.h"


GameScene::GameScene():_stage(0), _killCnt(0)
{
	ObjectManager.push_back(new FrameObject());
	PlayerObject *p = new PlayerObject(this);
	ObjectManager.push_back(p);
	ObjectManager.push_back(new HUDObject(p->getHPStore(),&_stage));

	initStage();
}

void GameScene::Update()
{
	for (auto iter = ObjectManager.begin(); iter != ObjectManager.end();)
	{
		if ((*iter)->Action())
		{
			iter++;
		}
		else
		{
			if ((*iter)->GetDataType() == DataType::PLAYER)
			{
				SceneManager::GetInstance()->LoadScene(SceneFlag::OVER);
				return;
			}
			else if ((*iter)->GetDataType() == DataType::ENEMY)
				_killCnt++;
			iter=ObjectManager.erase(iter);
		}
	}
	for (auto iter = ObjectManager.begin(); iter != ObjectManager.end(); iter++)
	{
		(*iter)->Draw();
	}

	if (clearCheck())
	{
		if(!initStage())
			SceneManager::GetInstance()->LoadScene(SceneFlag::OVER);
	}
}

bool GameScene::initStage()
{
	_stage++;
	_killCnt = 0;

	if (_stage > MAX_STAGE)
		return false;

	int iX = 7;
	int iY = 5;

	for (int i = 0; i < MAX_ENEMY; i++)
	{
		if (iX >= dfSCREEN_WIDTH - 8)
		{
			iX = 7;
			iY += 2;
		}

		ObjectManager.push_back(new EnemyObject(this, iX, iY, _stage));
		iX += 5;
	}

	return true;
}

bool GameScene::clearCheck()
{
	if (_killCnt >= MAX_ENEMY*3 / 8)
	{
		for (auto iter = ObjectManager.begin(); iter != ObjectManager.end();)
		{
			if ((*iter)->GetDataType() == DataType::BULLET || (*iter)->GetDataType() == DataType::ENEMY)
				iter = ObjectManager.erase(iter);
			else iter++;
		}
		score += CLEAR_SCORE*_stage;
		return true;
	}

	return false;
}