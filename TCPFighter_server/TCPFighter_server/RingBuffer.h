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