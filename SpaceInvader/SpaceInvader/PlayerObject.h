#pragma once
#include "BaseObject.h"
#define PLAYER_HP 15
//keyboard ют╥б
#define BIT_UP 0x01
#define BIT_DOWN 0x02
#define BIT_LEFT 0x04
#define BIT_RIGHT 0x08
#define BIT_SHOOT 0x10

#define CLASH_DAMAGE 10

class PlayerObject : public BaseObject
{
public:
	PlayerObject();
	PlayerObject(BaseScene *);
	bool Action();
	bool Draw();
	int *getHPStore() { return &_HP; }
private:
	int _HP;
	char _keybit;

	void keyboard();
	void keyEvent();
	void shootBullet();
	void hitCheck();
};