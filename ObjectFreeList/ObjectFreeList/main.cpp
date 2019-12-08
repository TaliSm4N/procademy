#include <iostream>
#include <Windows.h>
#include "ObjectFreeList.h"

class MyClass
{
private:
	int test;
public:
	int get() const { return test; }
	void set(int t) { test = t; }
};

int main()
{
	ObjectFreeList<MyClass> *test = new ObjectFreeList<MyClass>(0);
	printf("%d\n",sizeof(void *));

	MyClass *it1 = test->Alloc();
	MyClass *it2 = test->Alloc();
	MyClass *it3 = test->Alloc();
	MyClass *it4 = test->Alloc();
	MyClass *it5 = test->Alloc();
	MyClass *it6 = test->Alloc();

	it1->set(10);
	it2->set(20);
	it3->set(30);
	it4->set(40);
	it5->set(50);
	it6->set(60);


	printf("%d %d %d %d %d %d\n", it1->get(), it2->get(), it3->get(), it4->get(), it5->get(), it6->get());

	test->Free(it2);
	test->Free(it4);
	test->Free(it1);
	MyClass *it7 = test->Alloc();
	test->Free(it3);
	test->Free(it6);
	test->Free(it5);

	it1 = test->Alloc();
	it2 = test->Alloc();
	it3 = test->Alloc();
	it4 = test->Alloc();
	it5 = test->Alloc();
	it6 = test->Alloc();


	printf("%d %d %d %d %d %d %d\n", it1->get(), it2->get(), it3->get(), it4->get(), it5->get(), it6->get(),it7->get());

	system("pause");

	return 0;
}