#ifndef __MAKE_PACKET__
#define __MAKE_PACKET__

void MakePacketCreatePlayer(Packet &p, DWORD id, BYTE dir, WORD x, WORD y, BYTE HP);
void MakePacketCreateOtherPlayer(Packet &p, DWORD id, BYTE dir, WORD x, WORD y, BYTE HP);
void MakePacketRemovePlayer(Packet &p, DWORD id);
void MakePacketMoveStart(Packet &p, DWORD id, BYTE dir, WORD x, WORD y);
void MakePacketMoveStop(Packet &p, DWORD id, BYTE dir, WORD x, WORD y);
void MakePacketSync(Packet &p, DWORD id, WORD x, WORD y);
void MakePacketEcho(Packet &p, DWORD time);
void MakePacketAttack1(Packet &p, DWORD id, BYTE direction, WORD x, WORD y);
void MakePacketAttack2(Packet &p, DWORD id, BYTE direction, WORD x, WORD y);
void MakePacketAttack3(Packet &p, DWORD id, BYTE direction, WORD x, WORD y);
void MakePacketDamage(Packet &p, DWORD attackID, DWORD damageID, BYTE DamageHP);

#endif