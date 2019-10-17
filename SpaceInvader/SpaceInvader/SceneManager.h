#pragma once

#include "BaseScene.h"

enum SceneFlag { TITLE = 0, GAME, OVER };

class SceneManager
{
public:
	static SceneManager *GetInstance();

	void run();
	void LoadScene(SceneFlag f);
private:
	SceneManager();
	static SceneManager *_sManager;
	BaseScene *now;
	BaseScene *next;
};