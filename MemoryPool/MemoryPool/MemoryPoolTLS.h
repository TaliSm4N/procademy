#pragma once

#include "MemoryPool.h"

#define CHUNK_SIZE 200
#define BUMPER 0xffeeffee

template<class T>
class MemoryPoolTLS
{
private:
	struct Chunk;
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
	struct Chunk
	{
		ChunkBlock block[CHUNK_SIZE];
		int AllocIndex;
		int FreeCount;
		MemoryPool<Chunk> *pMemoryPool;

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
	T *Alloc();
	bool Free(T *data);
	int GetChunkCount() { return _useChunkCount; }
private:
	MemoryPool<Chunk> *_pool;
	int _useChunkCount;
	DWORD _tlsIndex;
	bool _placementNew;
};

template<class T>
MemoryPoolTLS<T>::MemoryPoolTLS(int ChunkNum,bool placement)
	:_useChunkCount(0)
{
	_tlsIndex = TlsAlloc();
	_pool = new MemoryPool<Chunk>(ChunkNum);
}

template<class T>
T *MemoryPoolTLS<T>::Alloc()
{
	Chunk *chunk = (Chunk *)TlsGetValue(_tlsIndex);
	T *ret;

	if (chunk == NULL)
	{
		chunk = _pool->Alloc(_placementNew);
		chunk->pMemoryPool = _pool;
		TlsSetValue(_tlsIndex, chunk);
	}
	else if (chunk->AllocIndex == CHUNK_SIZE)
	{
		chunk = _pool->Alloc(_placementNew);
		chunk->pMemoryPool = _pool;
		TlsSetValue(_tlsIndex, chunk);
	}

	ret = &chunk->block[chunk->AllocIndex].item;
	chunk->AllocIndex++;

	return ret;
}

template<class T>
bool MemoryPoolTLS<T>::Free(T *data)
{
	ChunkBlock *chunkBlock = (ChunkBlock *)((LONG64)data - sizeof(Chunk *));

	if (chunkBlock->_bumper != BUMPER)
	{
		return false;
	}
	Chunk *chunk = chunkBlock->pChunk;

	chunk->FreeCount++;

	if (chunk->FreeCount >= CHUNK_SIZE)
	{
		if (!chunk->pMemoryPool->Free(chunk))
		{
			return false;
		}
	}

	return true;
}