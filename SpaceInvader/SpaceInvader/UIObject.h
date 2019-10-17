#pragma once
#include "BaseObject.h"
#include "Screen.h"


class  StringObject : public BaseObject
{
public:
	StringObject(int x, int y, const char *str);
	bool Action();
	bool Draw();
private:
	char _str[64];
};

class FrameObject :public BaseObject
{
public:
	FrameObject();
	bool Action();
	bool Draw();
private:
	const char frame[dfSCREEN_HEIGHT][dfSCREEN_WIDTH]= {
	"------------------------------------------------------------",
	"|                                                          |",
	"|                                                          |",
	"|                                                          |",
	"|                                                          |",
	"|                                                          |",
	"|                                                          |",
	"|                                                          |",
	"|                                                          |",
	"|                                                          |",
	"|                                                          |",
	"|                                                          |",
	"|                                                          |",
	"|                                                          |",
	"|                                                          |",
	"|                                                          |",
	"|                                                          |",
	"|                                                          |",
	"|                                                          |",
	"|                                                          |",
	"|                                                          |",
	"|                                                          |",
	"|                                                          |",
	"|                                                          |",
	"|                                                          |",
	"|                                                          |",
	"|                                                          |",
	"|                                                          |",
	"|                                                          |",
	"|                                                          |",
	"|                                                          |",
	"|                                                          |",
	"|                                                          |",
	"|                                                          |",
	"|                                                          |",
	"|                                                          |",
	"|                                                          |",
	"|                                                          |",
	"|                                                          |",
	"------------------------------------------------------------"
	};
};

class HUDObject :public BaseObject
{
public:
	HUDObject(int *HP, int *stage) :_HP(HP), _stage(stage), BaseObject(DataType::UI, 0, 1) {}
	bool Action();
	bool Draw();
private:
	int *_HP;
	int *_stage;
};