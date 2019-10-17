#pragma once

#include "BaseScene.h"

#define MAX_STAGE 7
#define MAX_ENEMY 40

class GameScene : public BaseScene
{
public:
	void Update();
	GameScene();
private:
	int _stage;
	int _killCnt;

	bool initStage();
	bool clearCheck();
};