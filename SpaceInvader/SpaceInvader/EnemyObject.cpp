#include "EnemyObject.h"
#include "BulletObject.h"
#include "BaseScene.h"
#include "Screen.h"
#include "score.h"
#include <cstdlib>

EnemyObject::EnemyObject(BaseScene *sc,int x,int y,int stage) :_HP(ENEMY_HP),_stage(stage),_s_term(0),_m_term(0),_right_rotate(true), BaseObject(DataType::ENEMY, x, y, sc)
{
}

bool EnemyObject::Action()
{
	if(_s_term>=ENEMY_SHOOT_TERM-_stage+1)
		shoot();
	_s_term++;

	if (_m_term >= ENEMY_MOVE_TERM - _stage + 1)
		move();
	_m_term++;

	hitCheck();

	if (_HP > 0)
		return true;
	else
	{
		score += KILL_SCORE;
		return false;
	}
}

bool EnemyObject::move()
{
	_m_term = 0;
	if (_right_rotate)
	{
		if (_x < dfSCREEN_WIDTH - 3)
			_x++;
		else
		{
			_right_rotate = false;
			_y++;
		}
	}
	else
	{
		if (_x > 1)
			_x--;
		else
		{
			_right_rotate = true;
			_y++;
		}
	}

	if (_y >= dfSCREEN_HEIGHT - 2)
		return false;
	return true;
}

void EnemyObject::shoot()
{
	_s_term = 0;
	if (_y < dfSCREEN_HEIGHT - 2)
	{
		if (rand() % 100 < 2)
		{
			BaseObject *b = new BulletObject(E_BULLET, _x, _y + 1, _myScene);
			_myScene->getOM()->push_back(b);
		}
	}
}

bool EnemyObject::Draw()
{
	Screen::GetInstance()->draw(_x, _y, '#');
	return true;
}

void EnemyObject::hitCheck()
{
	auto temp = _myScene->getOM();
	for (auto iter = temp->begin(); iter != temp->end();)
	{
		if ((*iter)->GetDataType() == DataType::BULLET && ((BulletObject *)(*iter))->GetType() == P_BULLET)
		{
			if (((BulletObject *)(*iter))->_x == _x)
			{
				if (((BulletObject *)(*iter))->_y <= _y && ((BulletObject *)(*iter))->_y > _y + ((BulletObject *)(*iter))->_speed)
				{
					_HP -= ((BulletObject *)(*iter))->_damage;
					iter = temp->erase(iter);;
				}
				else iter++;
			}
			else iter++;
		}
		else iter++;
	}
}