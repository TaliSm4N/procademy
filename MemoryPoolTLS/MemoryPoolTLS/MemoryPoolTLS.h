#pragma once

#include "MemoryPool.h"
//#include "Profiler.h"

#define CHUNK_SIZE 200
#define BUMPER 0xffeeffee

//�ֱ� 50000���� ����Ѵ�
//�Ǿ� ����(16����) 1�϶� dequeue
//�Ǿ� ����(16����) 0�϶� enqueue
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
	class Chunk;
//#pragma pack(push,1)
	struct ChunkBlock
	{
		Chunk *pChunk;
		T item;
		int _bumper;
		ChunkBlock()
		{
			_bumper = BUMPER;
			pChunk = NULL;
		}
	};
//#pragma pack(pop)

	class Chunk
	{
	//private:
	public:
		bool used;
		int AllocIndex;
		LONG FreeCount;
		ChunkBlock block[CHUNK_SIZE];
		MemoryPoolTLS<T> *pMemoryPool;
		
	public:
		Chunk()
		{
			AllocIndex = 0;
			FreeCount = 0;
			used = false;
			//for (int i = 0; i < CHUNK_SIZE; i++)
			//{
			//	block[i].pChunk = this;
			//}
			
			//pMemoryPool = _pool;
		}

		void init()
		{
			used = true;
			for (int i = 0; i < CHUNK_SIZE; i++)
			{
				block[i].pChunk = this;
			}
		}

		T *Alloc()
		{
			T *ret = &block[AllocIndex].item;
			AllocIndex++;

			if (AllocIndex == CHUNK_SIZE)
			{
				pMemoryPool->chunkAlloc();
			}

			return ret;
		}

		bool Free()
		{
			if (InterlockedIncrement(&FreeCount) == CHUNK_SIZE)
			{
				if (!pMemoryPool->_pool.Free(this))
				{
					return false;
				}
			}

			return true;
		}
	};


public:
	MemoryPoolTLS(int ChunkNum = 0, bool placement=true);
	~MemoryPoolTLS();
	T *Alloc();
	bool Free(T *data);
	int GetChunkCount() { return _useChunkCount; }
private:
	Chunk *chunkAlloc()
	{
		Chunk *chunk = _pool.Alloc();

		if (chunk == NULL)
		{
			return NULL;
		}

		if (!chunk->used)
		{
			chunk->init();
		}
		else
		{
			volatile int test = 1;
		}

		chunk->pMemoryPool = this;
		TlsSetValue(_tlsIndex, chunk);

		return chunk;
	}
private:
	DWORD _tlsIndex;
	MemoryPool<Chunk> _pool;
	int _useChunkCount;
	bool _placementNew;
};

template<class T>
MemoryPoolTLS<T>::MemoryPoolTLS(int ChunkNum,bool placement)
	:_useChunkCount(0),_placementNew(placement)
{
	_tlsIndex = TlsAlloc();
	//_pool = new MemoryPool<Chunk>(ChunkNum);
}

template<class T>
MemoryPoolTLS<T>::~MemoryPoolTLS()
{
	//delete _pool;
}



template<class T>
T *MemoryPoolTLS<T>::Alloc()
{
	Chunk *chunk = (Chunk *)TlsGetValue(_tlsIndex);
	T *ret;

	if (chunk == NULL)
	{
		chunk = chunkAlloc();
		//PRO_BEGIN(L"CHUNK_ALLOC");
		//chunk = _pool.Alloc();//chunk�� �ݵ�� placement new
		//
		//if (chunk == NULL)
		//{
		//	return NULL;
		//}
		//
		////PRO_END(L"CHUNK_ALLOC");
		//chunk->pMemoryPool = this;
		//TlsSetValue(_tlsIndex, chunk);
		//InterlockedIncrement((LONG *)&_useChunkCount);
		//printf("set Chunk\n");
	}
	

	//printf("%d\n", chunk->AllocIndex);

	//ret = &(chunk->block[chunk->AllocIndex++].item);

	ret = chunk->Alloc();
	
	if (_placementNew)
	{
		//PRO_BEGIN(L"PLACE_NEW");
		new (ret) T();
		//PRO_END(L"PLACE_NEW");
	}

	//ret = chunk->Alloc();

	//if (chunk->AllocIndex == CHUNK_SIZE)
	//{
	//	//PRO_BEGIN(L"CHUNK_ALLOC");
	//	chunk = _pool.Alloc();//chunk�� �ݵ�� placement new
	//
	//	if (chunk == NULL)
	//	{
	//		return NULL;
	//	}
	//	//PRO_END(L"CHUNK_ALLOC");
	//	chunk->pMemoryPool = this;
	//	TlsSetValue(_tlsIndex, chunk);
	//	//InterlockedIncrement((LONG *)&_useChunkCount);
	//	//printf("newChunk\n");
	//}

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

	 return chunk->Free();

	//chunk->FreeCount++;
	//InterlockedIncrement((LONG *)&chunk->FreeCount);
	//
	//f (chunk->FreeCount == CHUNK_SIZE)
	//
	//	//�ش� ûũ�� Free�� ȣ���� memoryPool�� ������ �ʾ��� ��찡 ���� �� �����Ƿ�
	//	//chunk���� memoryPool�� ���� free�ؾ��Ѵ�.
	//
	//
	//	if (!chunk->pMemoryPool->_pool->Free(chunk))
	//	{
	//		return false;
	//	}
	//	//InterlockedDecrement((LONG *)&chunk->pMemoryPool->_useChunkCount);
	//

	//return true;
}