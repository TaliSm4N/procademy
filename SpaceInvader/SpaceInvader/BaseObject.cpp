#include "BaseObject.h"
#include "BaseScene.h"
BaseObject::BaseObject(int ObjectType, int x, int y) :_x(x), _y(y), _oType(ObjectType)
{
}

BaseObject::BaseObject(int ObjectType, int x, int y,BaseScene *sc) : _x(x), _y(y), _oType(ObjectType),_myScene(sc)
{
}