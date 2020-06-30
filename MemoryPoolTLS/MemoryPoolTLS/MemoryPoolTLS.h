#pragma once

#include "MemoryPool.h"

#define CHUNK_SIZE 200
#define BUMPER 0xffeeffee

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
	int GetCount() { return _useCount; }
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

		chunk->pMemoryPool = this;
		TlsSetValue(_tlsIndex, chunk);

		return chunk;
	}
private:
	DWORD _tlsIndex;
	MemoryPool<Chunk> _pool;
	bool _placementNew;
	int _useCount;
};

template<class T>
MemoryPoolTLS<T>::MemoryPoolTLS(int ChunkNum,bool placement)
	:_placementNew(placement)
{
	_tlsIndex = TlsAlloc();
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
	}

	ret = chunk->Alloc();
	
	if (_placementNew)
	{
		new (ret) T();
	}
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

	 InterlockedDecrement((LONG *)&_useCount);

	 return chunk->Free();
}