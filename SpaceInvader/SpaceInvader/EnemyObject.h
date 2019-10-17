#pragma once
#include "BaseObject.h"

#define ENEMY_HP 10
#define ENEMY_MOVE_TERM 5
#define ENEMY_SHOOT_TERM 8

class EnemyObject : public BaseObject
{
public:
	friend class PlayerObject;
	EnemyObject(BaseScene *sc, int x, int y,int stage);
	bool Action();
	bool Draw();
private:
	int _HP;
	int _stage;
	int _s_term;
	int _m_term;
	bool _right_rotate;

	bool move();
	void shoot();
	void hitCheck();
};