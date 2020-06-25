#ifndef __LUMO_PACKET__
#define __LUMO_PACKET__

#define DEFAULT_PACKET_SIZE 500

enum PacketType {LOCAL_TYPE = 0,NET_TYPE};
enum PacketMode {ERROR_MODE=0, THROW_MODE};


enum PacketError {E_NOERROR=0,E_PUTDATA_ERROR, E_GETDATA_ERROR};

#pragma pack(push,1)
typedef struct NetServerHeader
{
	BYTE code;
	WORD len;
	BYTE RandKey;
	BYTE CheckSum;

};
#pragma pack(pop)

typedef struct LanServerHeader
{
	WORD len;
};


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
	// ��Ŷ  �ı�.
	//
	// Parameters: ����.
	// Return: ����.
	//////////////////////////////////////////////////////////////////////////
	void Release(void);


	//////////////////////////////////////////////////////////////////////////
	// ��Ŷ û��.
	//
	// Parameters: ����.
	// Return: ����.
	//////////////////////////////////////////////////////////////////////////
	void Clear(void);


	//////////////////////////////////////////////////////////////////////////
	// ���� ������ ���.
	//
	// Parameters: ����.
	// Return: (int)��Ŷ ���� ������ ���.
	//////////////////////////////////////////////////////////////////////////
	int GetBufferSize(void) { return size-headerSize; }
	//////////////////////////////////////////////////////////////////////////
	// ���� ������� ������ ���.
	//
	// Parameters: ����.
	// Return: (int)������� ����Ÿ ������.
	//////////////////////////////////////////////////////////////////////////
	int GetDataSize(void) { return rear-front; }



	//////////////////////////////////////////////////////////////////////////
	// ���� ������ ���.
	//
	// Parameters: ����.
	// Return: (char *)���� ������.
	//////////////////////////////////////////////////////////////////////////
	char *GetBufferPtr(void) { return buf+front; }

	//send�� �����ʹ� front�� �׻� 0�̿�����
	void *GetSendPtr(void) { return buf; }
	
	char *GetHeaderPtr(void) { return buf; }


	//////////////////////////////////////////////////////////////////////////
	// ���� Pos �̵�. (�����̵��� �ȵ�)
	// GetBufferPtr �Լ��� �̿��Ͽ� �ܺο��� ������ ���� ������ ������ ��� ���. 
	//
	// Parameters: (int) �̵� ������.
	// Return: (int) �̵��� ������.
	//////////////////////////////////////////////////////////////////////////
	int MoveWritePos(int iSize);
	int MoveReadPos(int iSize);



	void encode();
	void decode();


	/* ============================================================================= */
	// ������ �����ε�
	/* ============================================================================= */
	Packet &operator = (Packet &clSrcPacket);

	//////////////////////////////////////////////////////////////////////////
	// �ֱ�. �� ���� Ÿ�Ը��� ��� ����.
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
	// ����. �� ���� Ÿ�Ը��� ��� ����.
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

	void GetHeader(char *desheader);
	void PutHeader(char *srcheader);


	//////////////////////////////////////////////////////////////////////////
	// ����Ÿ ���.
	//
	// Parameters: (char *)Dest ������. (int)Size.
	// Return: (int)������ ������.
	//////////////////////////////////////////////////////////////////////////
	int GetData(char *chpDest, int iSize);

	//////////////////////////////////////////////////////////////////////////
	// ����Ÿ ����.
	//
	// Parameters: (char *)Src ������. (int)SrcSize.
	// Return: (int)������ ������.
	//////////////////////////////////////////////////////////////////////////
	int PutData(char *chpSrc, int iSrcSize);

	bool VerifyCheckSum();


	//���Ź��� packet�� ��� encoding�� ��Ŷ���� �����ϱ� ����
	void RecvEncode() { encodeFlag = true; }


	//////////////////////////////////////////////////////////////////////////
	// ���� �ֱ� errorȮ��
	// Return: (int) �ֱ� �߻��� error �ڵ�
	//error 0 ���� ����
	//error 1 put data ����
	//error 2 get data ����
	//////////////////////////////////////////////////////////////////////////
	int GetLastError() const { return err; }

	static void Init(int key = 0,int code = 0);
	static Packet *Alloc(PacketType type = NET_TYPE);
	static bool Free(Packet *);

	static int PacketUseCount() { return packetPool->GetCount(); }
	static int GetCode() { return _code; }

	int GetHeaderSize() { return headerSize; }

	void SetDisconnect() { disconnectFlag = true; }
	bool GetDisconnectFlag() { return disconnectFlag; }

private:
	BYTE mode;
	int err;
	int size;
	int front;
	int rear;
	int refCnt;
	bool encodeFlag;
	bool disconnectFlag;

	int encodeCount;
	PacketType _type;
	int headerSize;

//#pragma pack(push,1)
	//HEADER header;
	char buf[DEFAULT_PACKET_SIZE];
//#pragma pack(pop)
	
	static int _key;
	static int _code;
	static MemoryPoolTLS<Packet> *packetPool;
};

#endif