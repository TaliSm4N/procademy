#pragma once
#include "BaseObject.h"

#define P_BULLET 0
#define E_BULLET 1

#define P_BULLET_SPEED 2
#define P_BULLET_DAMAGE 5
#define E_BULLET_SPEED 1
#define E_BULLET_DAMAGE 3

class BulletObject : public BaseObject
{
public:
	friend class EnemyObject;
	friend class PlayerObject;
	BulletObject(int type, int x, int y, BaseScene *sc);
	bool Action();
	bool Draw();
	int GetType() const { return _type; }
private:
	int _type;
	int _speed;
	int _damage;
};