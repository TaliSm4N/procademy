#ifndef __LUMO_RINGBUFFER__
#define __LUMO_RINGBUFFER__

#define RINGBUF_DEFAULT_SIZE 10000

class RingBuffer
{
public:
	RingBuffer();
	RingBuffer(int iBufferSize);
	~RingBuffer();

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


	int Enqueue(char *chpData, int iSize);
	int Enqueue(Packet &p);
	int Dequeue(char *chpData,int size);
	int Dequeue(Packet &p,int iSize);
	int Peek(char *chpData, int size);
	bool MoveFront(int size);
	bool MoveRear(int size);

	bool MoveWritePos(int size) { return MoveRear(size); }
	bool MoveReadPos(int size) { return MoveFront(size); }


	char *GetWritePos() const;
	char *GetReadPos() const;

	//int Put(char *chpData, int iSize);
	//int Get(char *chpData, int iSize);

};

#endif