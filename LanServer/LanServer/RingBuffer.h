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

private: 
	void Initial(int iBufferSize);

	bool isFull();
	bool isEmpty();

public:
	void Resize(int size);
	int GetBufferSize(void) const;

	void Reset();

	/////////////////////////////////////////////////////////////////////////
	// 현재 사용중인 용량 얻기.
	//
	// Parameters: 없음.
	// Return: (int)사용중인 용량.
	/////////////////////////////////////////////////////////////////////////
	int GetUseSize(void) const;

	/////////////////////////////////////////////////////////////////////////
	// 현재 버퍼에 남은 용량 얻기. 
	//
	// Parameters: 없음.
	// Return: (int)남은용량.
	/////////////////////////////////////////////////////////////////////////
	int GetFreeSize(void) const;

	/////////////////////////////////////////////////////////////////////////
	// 버퍼 포인터로 외부에서 한방에 읽고, 쓸 수 있는 길이.
	// (끊기지 않은 길이)
	//
	// 원형 큐의 구조상 버퍼의 끝단에 있는 데이터는 끝 -> 처음으로 돌아가서
	// 2번에 데이터를 얻거나 넣을 수 있음. 이 부분에서 끊어지지 않은 길이를 의미
	//
	// Parameters: 없음.
	// Return: (int)사용가능 용량.
	////////////////////////////////////////////////////////////////////////
	int DirectEnqueueSize(void) const;
	int DirectDequeueSize(void) const;

	//////////////////////////////////////////////////////////////////////////
	// 버퍼에 삽입 및 삭제
	// Parameters: Packet 또는 char 배열 + 배열의 크기
	// Returns : (int)삽입 또는 삭제된 데이터의 크기
	//////////////////////////////////////////////////////////////////////////
	int Enqueue(char *chpData, int iSize);
	int Dequeue(char *chpData,int size);
#ifdef __LUMO_PACKET__
	int Enqueue(Packet *p);
	int Dequeue(Packet *p,int iSize);
#endif
#ifdef _XSTRING_
	int Enqueue(std::wstring *str, int iSize);
	int Dequeue(std::wstring *str, int iSize);
#endif

	//////////////////////////////////////////////////////////////////////////
	// 버퍼에서 데이터 복제
	// Parameters: Packet 또는 char 배열 + 배열의 크기
	// Returns : (int)복제된 데이터의 크기
	//////////////////////////////////////////////////////////////////////////
	int Peek(char *chpData, int size);

	//////////////////////////////////////////////////////////////////////////
	// front와 rear를 이동
	// Parameters: 이동시킬 크기
	// Returns : (bool) 함수 성공 여부
	//////////////////////////////////////////////////////////////////////////
	bool MoveFront(int size);
	bool MoveRear(int size);
	bool MoveWritePos(int size) { return MoveRear(size); }
	bool MoveReadPos(int size) { return MoveFront(size); }

	//////////////////////////////////////////////////////////////////////////
	// read write하는 버퍼의 시작 포인터 구하기
	// Parameters: 없음
	// Returns : (char *) 버퍼의 위치
	//////////////////////////////////////////////////////////////////////////
	char *GetWritePos() const;
	char *GetReadPos() const;
	char *GetBufPtr() const;

	void Lock();
	void UnLock();
};

#endif