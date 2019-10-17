#pragma once

//#include "BaseScene.h"
class BaseScene;//상호 참조 방지

enum DataType { UI = 0, PLAYER, ENEMY, BULLET };

class BaseObject
{
public:
	BaseObject(int ObjectType, int x, int y);
	BaseObject(int ObjectType, int x, int y,BaseScene *);
	virtual ~BaseObject() {};

	virtual bool Action() = 0;
	virtual bool Draw() = 0;
	int GetDataType() { return _oType; };
protected:
	int _x;
	int _y;
	int _oType;
	BaseScene *_myScene;
};