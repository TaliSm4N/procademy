#ifndef __LUMO_RINGBUFFER__
#define __LUMO_RINGBUFFER__

#define RINGBUF_DEFAULT_SIZE 50000

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
	SRWLOCK srwLock;
	int srwCnt;

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

	//////////////////////////////////////////////////////////////////////////
	// ���ۿ� ���� �� ����
	// Parameters: Packet �Ǵ� char �迭 + �迭�� ũ��
	// Returns : (int)���� �Ǵ� ������ �������� ũ��
	//////////////////////////////////////////////////////////////////////////
	int Enqueue(char *chpData, int iSize);
	int Dequeue(char *chpData,int size);
#ifdef __LUMO_PACKET__
	int Enqueue(Packet &p);
	int Dequeue(Packet &p,int iSize);
#endif
#ifdef _XSTRING_
	int Enqueue(std::wstring &str, int iSize);
	int Dequeue(std::wstring &str, int iSize);
#endif

	//////////////////////////////////////////////////////////////////////////
	// ���ۿ��� ������ ����
	// Parameters: Packet �Ǵ� char �迭 + �迭�� ũ��
	// Returns : (int)������ �������� ũ��
	//////////////////////////////////////////////////////////////////////////
	int Peek(char *chpData, int size);

	//////////////////////////////////////////////////////////////////////////
	// front�� rear�� �̵�
	// Parameters: �̵���ų ũ��
	// Returns : (bool) �Լ� ���� ����
	//////////////////////////////////////////////////////////////////////////
	bool MoveFront(int size);
	bool MoveRear(int size);
	bool MoveWritePos(int size) { return MoveRear(size); }
	bool MoveReadPos(int size) { return MoveFront(size); }

	//////////////////////////////////////////////////////////////////////////
	// read write�ϴ� ������ ���� ������ ���ϱ�
	// Parameters: ����
	// Returns : (char *) ������ ��ġ
	//////////////////////////////////////////////////////////////////////////
	char *GetWritePos() const;
	char *GetReadPos() const;
	char *GetBufPtr() const;

	void Lock();
	void UnLock();
};

#endif