#pragma once

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h>
#include <Windows.h>
#include <DirectXMath.h>
using namespace DirectX;

enum KEY
{
	KEY_IDLE		= 0x00,
	KEY_RIGHT	= 0x01,
	KEY_LEFT		= 0x02,
	KEY_UP			= 0x03,
	KEY_DOWN	= 0x04,
	KEY_CTRL		= 0x05,
	KEY_ALT		= 0x06,
	KEY_SHIFT	= 0x07,
	KEY_SPACE	= 0x08,
	KEY_Z			= 0x09
};

// ��� �÷��̾� ���� 
enum PLAYER_NUM { Player1, Player2, Player3, Player4, Player5, Player6 };
// �÷��̾� �ִϸ��̼� ��ȣ
enum PLAYER_ANIMATION_NUM { IDLE, RUN };
// �÷��̾� ����
enum PLAYER_STATE { NONE, ICE, BREAK };
enum STATE_TYPE { Init, Run, Over };

struct ITEM
{
	bool Hammer;				// ��ġ
	bool SpecialHammer;	// Ư�� ��ġ
	bool Timer;					// �ð� ����
};

struct PLAYER
{
	bool Role; 					// �÷��̾���� ����
	XMFLOAT3 Pos;			// �÷��̾� ��ġ
	XMFLOAT4 Dir;				// ����(���ʹϾ�)
	byte AnimationNum; 	// �ִϸ��̼� ��ȣ
	bool ItemUse;				// ������ ��� ����
	ITEM ItemType;				// ������ ����
	byte PlayerState;			// �÷��̾� ����
	u_short Score;				// �÷��̾� ����
};

// ��Ŷ ����
#pragma pack(1)

// [Ŭ�� -> ����]
struct CS_InitPacket
{
};

struct CS_InGamePacket
{
	byte PlayerID;	// �÷��̾� ��ȣ
	byte Key;			// Ű ��ư
};

// ==========================================

// [���� -> Ŭ��]
struct SC_InitPacket
{
	XMFLOAT3 ObjPos;	// ������Ʈ�� ��ġ
	XMFLOAT4 Dir;		// ����(���ʹϾ�)
};

struct SC_InGamePacket
{
	PLAYER		Player_Packet[6];
	byte			RoundCount;			// �� ��������
	float			Timer;						// ���� �ð�
	bool			Bomb;						// ��ź�� ��������
};
#pragma pack()