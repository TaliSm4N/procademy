#include "MMOLib.h"
#include "CommonProtocol.h"
#include "player.h"

Player::Player()
{
}

void Player::OnAuth_ClientJoin()
{
	//�÷��̾� �������� ���� ���ӻ��� ������ �Ҵ� �� �غ�
}
void Player::OnAuth_ClientLeave()
{
	//auth������ ����� ������ ����
}
void Player::OnAuth_Packet(Packet *p)
{
	//�α��� ��Ŷ Ȯ�� �� DB������ �ε�
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
	//���� ������ ������ ���� ������ �غ� �� ����
}
void Player::OnGame_ClientLeave()
{
	//���� ���������� �÷��̾� ����
}
void Player::OnGame_ClientRelease()
{
	_AccountNo = 0;
	//��������
}
void Player::OnGame_Packet(Packet *p)
{
	//���� ���� ��Ŷ ó��	

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