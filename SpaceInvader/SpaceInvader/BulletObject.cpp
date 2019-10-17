#include "BulletObject.h"
#include "BaseScene.h"
#include "Screen.h"

BulletObject::BulletObject(int type,int x, int y, BaseScene *sc) :_type(type),BaseObject(DataType::BULLET, x, y, sc)
{
	if (type == P_BULLET)
	{
		_speed = -P_BULLET_SPEED;
		_damage = P_BULLET_DAMAGE;
		_y += P_BULLET_SPEED;
	}
	else
	{
		_speed = E_BULLET_SPEED;
		_damage = E_BULLET_DAMAGE;
		_y -= E_BULLET_SPEED;
	}
}

bool BulletObject::Action()
{
	_y += _speed;

	if (_y >= dfSCREEN_HEIGHT - 2 || _y <= 1)
		return false;
	return true;
}

bool BulletObject::Draw()
{
	if (_type == P_BULLET)
		Screen::GetInstance()->draw(_x, _y, '^');
	else
		Screen::GetInstance()->draw(_x, _y, 'V');
	return true;
}