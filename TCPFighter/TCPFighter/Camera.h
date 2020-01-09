#pragma once


class Camera
{
public:
	Camera(int w, int h):width(w),height(h),_x(0),_y(0)
	{
	}

	int TransX(int x);
	int TransY(int y);
	int GetX() const { return _x; }
	int GetY() const { return _y; }
	void SetX(int x);// { _x = x; }
	void SetY(int y);// { _y = y; }
private:
	int _x;
	int _y;
	int width;
	int height;
};