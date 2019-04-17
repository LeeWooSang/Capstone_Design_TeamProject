#pragma once
#include <iostream>
#include <windows.h>
#include <DirectXMath.h>

using namespace std;
using namespace DirectX;
//<< Init ��Ŷ ���� >>

//[Ŭ��->����]
struct CS_INIT_PACKET
{

};

///////////////////////////////////////////////////////

//[����->Ŭ��] // �Ŷ��尡 ������ ������ �����ִ� ��Ŷ
struct SC_INIT_PACKET
{
	byte myId;
	XMFLOAT3 ObjPos;   // ������Ʈ�� ��ġ
	XMFLOAT4 Dir;      // ����(���ʹϾ�)
	unsigned short Score;     // �÷��̾� ����
};

//////////////////////////////////////////////////////

//<< InGame ��Ŷ ���� >>

//[Ŭ��->����]
struct CS_INGAME_PACKET
{
	byte PlayerNum;   // �÷��̾� ��ȣ
	byte Key;      // Ű ��ư
};

//////////////////////////////////////////////////////

//[����->Ŭ��]
struct SC_INGAME_PACKET
{
	byte id;
	PLAYER Player_Packet;
	byte RoundCount;      // �� ��������
	float Timer;      // ���� �ð�
	bool Bomb;      // ��ź�� ��������
};

enum ITEM { NONE = 0, HAMMER, GOLD_HAMMER, GOLD_TIMER, BOMB };
enum PLAYER_NUM { P1, P2, P3, P4, P5, P6 }; // ��� �÷��̾� ���� 
enum PLAYER_ANIMATION_NUM {
	Idle, Jump, RunFast,
	RunBackWard, ATK3, Digging, Die2, RaiseHand, Ice, Victory, Aert, Slide
}; // �÷��̾� �ִϸ��̼� ��ȣ
enum PLAYER_STATE { NONE, ICE, BREAK }; // �÷��̾� ����
enum STATE_TYPE { Init, Run, Over };

struct PLAYER
{
	bool isBomber;        	 // �÷��̾���� ����
	XMFLOAT3 Pos; 	             // �÷��̾� ��ġ
	XMFLOAT4 Dir;      	// ����(���ʹϾ�)
	byte AnimationNum;    	// �ִϸ��̼� ��ȣ
	float AnimationTime; 	// �ִϸ��̼� �ð� ����
	byte UsedItem;      	// ���Ǵ� ������ ����
	byte PlayerState;  	// �÷��̾� ����
};


//////////////////////////////////////////////////////
