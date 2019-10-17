#include "PlayerObject.h"
#include "EnemyObject.h"
#include "BulletObject.h"
#include "BaseScene.h"
#include "Screen.h"
#include "score.h"

PlayerObject::PlayerObject():_HP(PLAYER_HP),_keybit(0),BaseObject(DataType::PLAYER, dfSCREEN_WIDTH / 2, dfSCREEN_HEIGHT - 2)
{
}

PlayerObject::PlayerObject(BaseScene *sc) :_HP(PLAYER_HP), _keybit(0), BaseObject(DataType::PLAYER, dfSCREEN_WIDTH / 2, dfSCREEN_HEIGHT - 2,sc)
{
}

bool PlayerObject::Action()
{
	keyboard();
	keyEvent();
	hitCheck();

	if (_HP > 0)
		return true;
	else
		return false;
}

bool PlayerObject::Draw()
{
	Screen::GetInstance()->draw(_x, _y, 'A');
	return true;
}

void PlayerObject::keyboard()
{
	if (GetAsyncKeyState(VK_UP)&&_y>2)
		_keybit |= BIT_UP;
	if (GetAsyncKeyState(VK_DOWN) && _y < dfSCREEN_HEIGHT - 2)
		_keybit |= BIT_DOWN;
	if (GetAsyncKeyState(VK_LEFT) && _x > 1)
		_keybit |= BIT_LEFT;
	if (GetAsyncKeyState(VK_RIGHT) && _x < dfSCREEN_WIDTH - 3)
		_keybit |= BIT_RIGHT;
	if (GetAsyncKeyState('Z'))
		_keybit |= BIT_SHOOT;
}

void PlayerObject::keyEvent()
{
	if (_keybit&BIT_UP)
		_y--;
	if (_keybit&BIT_DOWN)
		_y++;
	if (_keybit&BIT_LEFT)
		_x--;
	if (_keybit&BIT_RIGHT)
		_x++;
	if (_keybit&BIT_SHOOT)
		shootBullet();
	_keybit = 0;
}

void PlayerObject::shootBullet()
{
	BaseObject *b = new BulletObject(P_BULLET, _x, _y - 1, _myScene);
	_myScene->getOM()->push_back(b);

	if (score > 0)
		score += SHOOT_SCORE;
}

void PlayerObject::hitCheck()
{
	auto temp = _myScene->getOM();
	for (auto iter = temp->begin(); iter != temp->end();)
	{
		if ((*iter)->GetDataType() == DataType::ENEMY)
		{
			if (_x == ((EnemyObject *)(*iter))->_x && _y == ((EnemyObject *)(*iter))->_y)
			{
				_HP -= CLASH_DAMAGE;
				iter = temp->erase(iter);
			}
			else iter++;
		}
		else if ((*iter)->GetDataType() == DataType::BULLET && ((BulletObject *)(*iter))->GetType() == E_BULLET)
		{
			if (((BulletObject *)(*iter))->_x == _x)
			{
				if (_keybit&BIT_UP)
				{
					if (((BulletObject *)(*iter))->_y - 1 <= _y && ((BulletObject *)(*iter))->_y + ((BulletObject *)(*iter))->_speed > _y)
					{
						_HP -= ((BulletObject *)(*iter))->_damage;
						iter = temp->erase(iter);;
					}
					else iter++;
				}
				else
				{
					if (((BulletObject *)(*iter))->_y <= _y && ((BulletObject *)(*iter))->_y + ((BulletObject *)(*iter))->_speed > _y)
					{
						_HP -= ((BulletObject *)(*iter))->_damage;
						iter = temp->erase(iter);;
					}
					else iter++;
				}
			}
			else iter++;
		}
		else iter++;
	}
}