#include "MMOLib.h"
#include "CommonProtocol.h"
#include "player.h"

Player::Player()
{
}

void Player::OnAuth_ClientJoin()
{
	//플레이어 접소으로 인한 게임상의 데이터 할당 및 준비
}
void Player::OnAuth_ClientLeave()
{
	//auth에서만 사용한 데이터 정리
}
void Player::OnAuth_Packet(Packet *p)
{
	//로그인 패킷 확인 및 DB데이터 로딩
	WORD type;

	*p >> type;

	switch (type)
	{
	case en_PACKET_CS_GAME_REQ_LOGIN:
		Auth_ReqLogin(p);
		break;
	default:
		Disconnect();
	}
}
void Player::OnGame_ClientJoin()
{
	//게임 컨텐츠 진입을 위한 데이터 준비 및 세팅
}
void Player::OnGame_ClientLeave()
{
	//게임 컨텐츠상의 플레이어 정리
}
void Player::OnGame_ClientRelease()
{
	_AccountNo = 0;
	//접속해제
}
void Player::OnGame_Packet(Packet *p)
{
	//실제 게임 패킷 처리	

	WORD type;

	*p >> type;

	switch (type)
	{
	case en_PACKET_CS_GAME_REQ_ECHO:
		Game_Echo(p);
		break;
	default:
		Disconnect();
	}
}

void Player::Auth_ReqLogin(Packet *p)
{
	Packet *sendPacket = Packet::Alloc();

	INT64 accountNo;
	BYTE status;

	*p >> accountNo;

	_AccountNo = accountNo;

	_bAuthToGameFlag = true;

	status = 1;

	*sendPacket << (WORD)en_PACKET_CS_GAME_RES_LOGIN << status << accountNo;

	SendPacket(sendPacket);


}
void Player::Game_Echo(Packet *p)
{
	Packet *sendPacket = Packet::Alloc();
	INT64 AccountNo;
	LONGLONG SendTick;

	*p >> AccountNo >> SendTick;

	if (AccountNo != _AccountNo)
	{
		volatile int test = 1;
	}

	*sendPacket << (WORD)en_PACKET_CS_GAME_RES_ECHO << AccountNo << SendTick;

	SendPacket(sendPacket);
}