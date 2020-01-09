#include "Camera.h"
#include "PlayerObject.h"

//#define dfRANGE_MOVE_TOP	0
//#define dfRANGE_MOVE_LEFT	0
//#define dfRANGE_MOVE_RIGHT	6400
//#define dfRANGE_MOVE_BOTTOM	6400

int Camera::TransX(int x)
{
	return width / 2 + x - _x;
}

int Camera::TransY(int y)
{
	return height / 2 + y - _y;
}

void Camera::SetX(int x) 
{ 
	if (width / 2 > x)
		_x = width / 2;
	else if (dfRANGE_MOVE_RIGHT - width / 2 < x)
		_x = dfRANGE_MOVE_RIGHT - width / 2;
	else
		_x = x; 
}

void Camera::SetY(int y) 
{ 
	if (height / 2 > y)
		_y = height / 2;
	else if (dfRANGE_MOVE_BOTTOM - height / 2 < y)
		_y = dfRANGE_MOVE_BOTTOM - height / 2;
	else
		_y = y; 
}