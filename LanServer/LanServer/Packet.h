#ifndef __LUMO_PACKET__
#define __LUMO_PACKET__

#define DEFAULT_PACKET_SIZE 500



enum PacketMode {ERROR_MODE=0, THROW_MODE};


enum PacketError {E_NOERROR=0,E_PUTDATA_ERROR, E_GETDATA_ERROR};

class Packet
{
public:
	Packet();
	Packet(int iBufferSize);
	Packet(int iBufferSize,int Mode);

	virtual ~Packet();


	void Ref();
	bool UnRef();
	//////////////////////////////////////////////////////////////////////////
	// 패킷  파괴.
	//
	// Parameters: 없음.
	// Return: 없음.
	//////////////////////////////////////////////////////////////////////////
	void Release(void);


	//////////////////////////////////////////////////////////////////////////
	// 패킷 청소.
	//
	// Parameters: 없음.
	// Return: 없음.
	//////////////////////////////////////////////////////////////////////////
	void Clear(void);


	//////////////////////////////////////////////////////////////////////////
	// 버퍼 사이즈 얻기.
	//
	// Parameters: 없음.
	// Return: (int)패킷 버퍼 사이즈 얻기.
	//////////////////////////////////////////////////////////////////////////
	int GetBufferSize(void) { return size; }
	//////////////////////////////////////////////////////////////////////////
	// 현재 사용중인 사이즈 얻기.
	//
	// Parameters: 없음.
	// Return: (int)사용중인 데이타 사이즈.
	//////////////////////////////////////////////////////////////////////////
	int GetDataSize(void) { return rear-front; }



	//////////////////////////////////////////////////////////////////////////
	// 버퍼 포인터 얻기.
	//
	// Parameters: 없음.
	// Return: (char *)버퍼 포인터.
	//////////////////////////////////////////////////////////////////////////
	char *GetBufferPtr(void) { return buf+front; }

	//send할 데이터는 front가 항상 0이여야함
	void *GetSendPtr(void) { return &header; }

	HEADER *GetHeaderPtr(void) { return &header; }

	//////////////////////////////////////////////////////////////////////////
	// 버퍼 Pos 이동. (음수이동은 안됨)
	// GetBufferPtr 함수를 이용하여 외부에서 강제로 버퍼 내용을 수정할 경우 사용. 
	//
	// Parameters: (int) 이동 사이즈.
	// Return: (int) 이동된 사이즈.
	//////////////////////////////////////////////////////////////////////////
	int MoveWritePos(int iSize);
	int MoveReadPos(int iSize);






	/* ============================================================================= */
	// 연산자 오버로딩
	/* ============================================================================= */
	Packet &operator = (Packet &clSrcPacket);

	//////////////////////////////////////////////////////////////////////////
	// 넣기. 각 변수 타입마다 모두 만듬.
	//////////////////////////////////////////////////////////////////////////
	Packet &operator << (BYTE byValue);
	Packet &operator << (char chValue);

	Packet &operator << (short shValue);
	Packet &operator << (WORD wValue);

	Packet &operator << (int iValue);
	Packet &operator << (DWORD dwValue);
	Packet &operator << (float fValue);

	Packet &operator << (__int64 iValue);
	Packet &operator << (double dValue);
	Packet &operator << (UINT64 iValue);
	Packet &operator << (UINT iValue);


	//////////////////////////////////////////////////////////////////////////
	// 빼기. 각 변수 타입마다 모두 만듬.
	//////////////////////////////////////////////////////////////////////////
	Packet &operator >> (BYTE &byValue);
	Packet &operator >> (char &chValue);

	Packet &operator >> (short &shValue);
	Packet &operator >> (WORD &wValue);

	Packet &operator >> (int &iValue);
	Packet &operator >> (DWORD &dwValue);
	Packet &operator >> (float &fValue);

	Packet &operator >> (__int64 &iValue);
	Packet &operator >> (double &dValue);
	Packet &operator >> (UINT64 &iValue);
	Packet &operator >> (UINT &iValue);

	void GetHeader(HEADER *desheader);
	void PutHeader(HEADER *srcheader);


	//////////////////////////////////////////////////////////////////////////
	// 데이타 얻기.
	//
	// Parameters: (char *)Dest 포인터. (int)Size.
	// Return: (int)복사한 사이즈.
	//////////////////////////////////////////////////////////////////////////
	int GetData(char *chpDest, int iSize);

	//////////////////////////////////////////////////////////////////////////
	// 데이타 삽입.
	//
	// Parameters: (char *)Src 포인터. (int)SrcSize.
	// Return: (int)복사한 사이즈.
	//////////////////////////////////////////////////////////////////////////
	int PutData(char *chpSrc, int iSrcSize);


	//////////////////////////////////////////////////////////////////////////
	// 가장 최근 error확인
	// Return: (int) 최근 발생한 error 코드
	//error 0 에러 없음
	//error 1 put data 에러
	//error 2 get data 에러
	//////////////////////////////////////////////////////////////////////////
	int GetLastError() const { return err; }

	static void Init();
	static Packet *Alloc();
	static bool Free(Packet *);

	static int PacketUseCount() { return packetPool->GetCount(); }

private:
	BYTE mode;
	int err;
	int size;
	int front;
	int rear;
	int refCnt;
	HEADER header;
	char buf[DEFAULT_PACKET_SIZE];

	static MemoryPoolTLS<Packet> *packetPool;
};

#endif