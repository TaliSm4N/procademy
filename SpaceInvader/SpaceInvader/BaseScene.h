#pragma once

//#include "CList.h"
#include <list>
//#include "BaseObject.h"

class BaseObject;//��ȣ ���� ���� 

class BaseScene
{
public:
	~BaseScene() 
	{
		ObjectManager.clear();
	}
	virtual void Update() = 0;
	//CList<BaseObject *> *getOM() { return &ObjectManager; }
	std::list<BaseObject *> *getOM() { return &ObjectManager; }
protected:
	//CList<BaseObject *>ObjectManager;
	std::list<BaseObject *>ObjectManager;
};