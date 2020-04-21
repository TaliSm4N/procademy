#pragma once

#include "MemoryPool.h"
//#include "Profiler.h"

#define CHUNK_SIZE 200
#define BUMPER 0xffeeffee

//최근 50000개만 기록한다
//맨앞 숫자(16진수) 1일때 dequeue
//맨앞 숫자(16진수) 0일때 enqueue
//#define TRACK_MAX 100000
//
//void *track[TRACK_MAX];
//int trackAlloc[TRACK_MAX];
//unsigned long long trackCur = 0;
//
//#define TRACK_MAX 100000
//
//void *trackBlock[TRACK_MAX];
//unsigned long long trackBlockCur = 0;


template <class T>
class MemoryPoolTLS
{
private:
	struct Chunk;
//#pragma pack(push,1)
	struct ChunkBlock
	{
		Chunk *pChunk;
		T item;
		int _bumper;
		ChunkBlock()
		{
			_bumper = BUMPER;
		}
	};
//#pragma pack(pop)

	struct Chunk
	{
		int AllocIndex;
		int FreeCount;
		ChunkBlock block[CHUNK_SIZE];
		MemoryPoolTLS<T> *pMemoryPool;

		Chunk()
		{
			for (int i = 0; i < CHUNK_SIZE; i++)
			{
				block[i].pChunk = this;
			}
			AllocIndex = 0;
			FreeCount = 0;
			//pMemoryPool = _pool;
		}

	};


public:
	MemoryPoolTLS(int ChunkNum = 0, bool placement=true);
	~MemoryPoolTLS();
	T *Alloc();
	bool Free(T *data);
	int GetChunkCount() { return _useChunkCount; }
	int GetCount() { return _useCount; }
private:
	DWORD _tlsIndex;
	MemoryPool<Chunk> *_pool;
	int _useChunkCount;
	bool _placementNew;
	int _useCount;
};

template<class T>
MemoryPoolTLS<T>::MemoryPoolTLS(int ChunkNum,bool placement)
	:_useChunkCount(0),_placementNew(placement)
{
	_tlsIndex = TlsAlloc();
	_pool = new MemoryPool<Chunk>(ChunkNum/200 + 1);//chunk 갯수 지정
}

template<class T>
MemoryPoolTLS<T>::~MemoryPoolTLS()
{
	delete _pool;
}

template<class T>
T *MemoryPoolTLS<T>::Alloc()
{
	__declspec(thread) static Chunk *beforeChunk = NULL;
	__declspec(thread) static int before = 0;
	__declspec(thread) static int checker = 0;

	Chunk *chunk = (Chunk *)TlsGetValue(_tlsIndex);
	T *ret;
	

	ChunkBlock *test;

	

	if (chunk == NULL)
	{
		//PRO_BEGIN(L"CHUNK_ALLOC");
		beforeChunk = chunk;
		chunk = _pool->Alloc();//chunk는 반드시 placement new

		if (chunk == NULL)
		{
			return NULL;
		}

		//PRO_END(L"CHUNK_ALLOC");
		chunk->pMemoryPool = this;
		TlsSetValue(_tlsIndex, chunk);
		//InterlockedIncrement((LONG *)&_useChunkCount);
		//printf("set Chunk\n");
	}
	int temp = chunk->AllocIndex;
	

	//printf("%d\n", chunk->AllocIndex);

	ret = &(chunk->block[chunk->AllocIndex++].item);
	
	if (_placementNew)
	{
		//PRO_BEGIN(L"PLACE_NEW");
		new (ret) T();
		//PRO_END(L"PLACE_NEW");
	}

	//ret = chunk->Alloc();

	if (chunk->AllocIndex > CHUNK_SIZE)
	{
		volatile int test = 1;
	}

	checker = chunk->AllocIndex;

	if (chunk->AllocIndex == CHUNK_SIZE)
	{
		//PRO_BEGIN(L"CHUNK_ALLOC");
		beforeChunk = chunk;
		chunk = _pool->Alloc();//chunk는 반드시 placement new

		if (chunk == NULL)
		{
			return NULL;
		}
		//PRO_END(L"CHUNK_ALLOC");
		chunk->pMemoryPool = this;
		TlsSetValue(_tlsIndex, chunk);
		//InterlockedIncrement((LONG *)&_useChunkCount);
		//printf("newChunk\n");
	}

	before = chunk->AllocIndex;
	InterlockedIncrement((LONG *)&_useCount);
	return ret;
}

template<class T>
bool MemoryPoolTLS<T>::Free(T *data)
{
	ChunkBlock *chunkBlock = (ChunkBlock *)((LONG64)data - sizeof(Chunk *));
	Chunk *chunk;

	if (chunkBlock->_bumper != BUMPER)
	{
		return false;
	}


	 chunk = chunkBlock->pChunk;

	//chunk->FreeCount++;
	 int tempFreeCount = InterlockedIncrement((LONG *)&chunk->FreeCount);

	if (tempFreeCount == CHUNK_SIZE)
	{
		//해당 청크가 Free를 호출한 memoryPool에 속하지 않았을 경우가 있을 수 있으므로
		//chunk내의 memoryPool을 통해 free해야한다.


		if (!chunk->pMemoryPool->_pool->Free(chunk))
		{
			return false;
		}
		//InterlockedDecrement((LONG *)&chunk->pMemoryPool->_useChunkCount);
	}
	InterlockedDecrement((LONG *)&_useCount);
	return true;
}