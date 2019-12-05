#pragma once

#define DEFAULT_SIZE 100
#include "Packet.h"

class RingBuffer
{
public:

	RingBuffer(int iBufferSize=DEFAULT_SIZE);

private:
	int _size;
	int _front;
	int _rear;
	int _capacity;
	char *_buf;

private: 
	void Initial(int iBufferSize);

	bool isFull();
	bool isEmpty();

public:
	void Resize(int size);
	int GetBufferSize(void) const;

	/////////////////////////////////////////////////////////////////////////
	// ���� ������� �뷮 ���.
	//
	// Parameters: ����.
	// Return: (int)������� �뷮.
	/////////////////////////////////////////////////////////////////////////
	int GetUseSize(void) const;

	/////////////////////////////////////////////////////////////////////////
	// ���� ���ۿ� ���� �뷮 ���. 
	//
	// Parameters: ����.
	// Return: (int)�����뷮.
	/////////////////////////////////////////////////////////////////////////
	int GetFreeSize(void) const;

	/////////////////////////////////////////////////////////////////////////
	// ���� �����ͷ� �ܺο��� �ѹ濡 �а�, �� �� �ִ� ����.
	// (������ ���� ����)
	//
	// ���� ť�� ������ ������ ���ܿ� �ִ� �����ʹ� �� -> ó������ ���ư���
	// 2���� �����͸� ��ų� ���� �� ����. �� �κп��� �������� ���� ���̸� �ǹ�
	//
	// Parameters: ����.
	// Return: (int)��밡�� �뷮.
	////////////////////////////////////////////////////////////////////////
	int DirectEnqueueSize(void) const;
	int DirectDequeueSize(void) const;


	/////////////////////////////////////////////////////////////////////////
	// WritePos �� ����Ÿ ����.
	//
	// Parameters: (char *)����Ÿ ������. (int)ũ��. 
	// Return: (int)���� ũ��.
	/////////////////////////////////////////////////////////////////////////
	int Enqueue(char *chpData, int iSize);
	int Enqueue(Packet &p);
	int Dequeue(char *chpData,int size);
	int Dequeue(Packet &p,int iSize);
	int Peek(char *chpData, int size);
	bool MoveFront(int size);
	bool MoveRear(int size);

	/////////////////////////////////////////////////////////////////////////
	// ReadPos ���� ����Ÿ ������. ReadPos �̵�.

	char *GetWritePos() const;
	char *GetReadPos() const;

};