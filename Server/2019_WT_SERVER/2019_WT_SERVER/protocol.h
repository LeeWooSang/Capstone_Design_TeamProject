#pragma once
#include <iostream>
//#include <windows.h>
//#include <DirectXMath.h>

using namespace std;
//using namespace DirectX;

enum ITEM { NONE = 0, HAMMER, GOLD_HAMMER, GOLD_TIMER, BOMB };
enum PLAYER_NUM { P1, P2, P3, P4, P5, P6 };						// ��� �÷��̾� ���� 
enum PLAYER_STATE { NONE, ICE, BREAK };							// �÷��̾� ����
enum STATE_TYPE { Init, Run, Over };

//[Ŭ��->����]
struct CS_INIT_PACKET
{

};

///////////////////////////////////////////////////////

//[����->Ŭ��] // �Ŷ��尡 ������ ������ �����ִ� ��Ŷ
//struct SC_INIT_PACKET
//{
//	char type;
//	char myId;
//	char xPos;		// ������Ʈ�� ��ġ
//	char yPos;
//	char zPos;
//	char xDir;		// ����(���ʹϾ�)
//	char yDir;
//	char zDir;
//	char wDir;
//	char score;		// �÷��̾� ����
//};

struct SC_ACCESS_COMPLETE
{
	char type;
	char myId;
	char score;				// �÷��̾� ����
	char roundCount;		// �� ��������
	char serverTime;				// ���� �ð�
};

struct SC_PUT_PLAYER
{
	char type;
	char myId;
	char xPos;		// ������Ʈ�� ��ġ
	char yPos;
	char zPos;
	char xDir;		// ����(���ʹϾ�)
	char yDir;
	char zDir;
	char wDir;
	char score;		// �÷��̾� ����
};

//////////////////////////////////////////////////////

//<< InGame ��Ŷ ���� >>

//[Ŭ��->����]
struct CS_RIGHT_KEY
{
	char type;
	char id;		// �÷��̾� ��ȣ
};

struct CS_LEFT_KEY
{
	char type;
	char id;		// �÷��̾� ��ȣ
};

struct CS_UP_KEYINGA
{
	char type;
	char id;		// �÷��̾� ��ȣ
};

struct CS_DOWN_KEY
{
	char type;
	char id;		// �÷��̾� ��ȣ
};

struct CS_BOMBER_TOUCH
{
	char type;
	char id;
	char touchedId;	// ��ġ�� �÷��̾� ��ȣ
};

//////////////////////////////////////////////////////

//[����->Ŭ��]
// ��ź �Ѱ��ִ°� ��� �����ߴ����� ���� isBomber ������ ��� ����ü ����
// ���� Ŭ��� ���� �ð����� �����������ϰ� �ִµ� ���� �ð��� �����Ӹ��� �����ִ°� ������?
// -> �ǿ������� ����� timer ������ ��� ����ü ����

// isBomber�� Ű �Է� �� �浹üũ�Ͽ� ������ �Ѿ����
// animNum �� animTime�� �� Ŭ�� �˾Ƽ� �ϵ���
// �� ���� ������ ���尡 ������ �� �� ���� ������ ��
// isBoomed�� ���� �ð��� �������
struct SC_INGAME_PACKET
{
	//char id;
	//char isBomber;
	//char xPos;
	//char yPos;
	//char zPos;
	//char xDir;
	//char yDir;
	//char zDir;
	//char wDir;
	//
	//char animNum;			// �ִϸ��̼� ��ȣ
	//char animTime;			// �ִϸ��̼� �ð� ����
	//char playerState;		// �÷��̾� ����

	//char usedItem;			// ���Ǵ� ������ ����
	//char roundCount;		// �� ��������
	//char timer;				// ���� �ð�
	//char isBoomed;			// ��ź�� ��������

	// parameter ������
	/*SC_INGAME_PACKET(char _id, char _isBomber, char _xPos, char _yPos, char _zPos, char _xDir, char _yDir, char _zDir, char _wDir,
		char _animNum, char _animTime, char _usedItem, char _playerState, char _roundCount, char _timer, char _isBoomed) :
		id(_id), isBomber(_isBomber), xPos(_xPos), yPos(_yPos), zPos(_zPos), xDir(_xDir), yDir(_yDir), zDir(_zDir), wDir(_wDir),
		animNum(_animNum), animTime(_animTime), usedItem(_usedItem), playerState(_playerState), roundCount(_roundCount), timer(_timer),
		isBoomed(_isBoomed) {};*/
};

// �÷��̾� �̵� ��
struct SC_MOVE_PLAYER
{
	char type;
	char id;
	char xPos;
	char yPos;
	char zPos;
	char xDir;
	char yDir;
	char zDir;
	char wDir;
};

// �÷��̾ ������ ��� ��
struct SC_USE_ITEM
{
	char type;
	char id;
	char usedItem;			// ���Ǵ� ������ ����
};

// ���� �������� �����ð��� Ŭ��ð��� ���ϱ� ����
// �ٸ� ��� Ŭ��ð��� �����ð����� �缳��
struct SC_COMPARE_TIME
{
	char type;
	char serverTime;				// ���� �ð�
};

struct SC_ROLL_CHANGE
{
	char type;
	char bomberId;
	char normalId;
};

struct SC_ROUND_END
{
	char type;
};

struct PLAYER
{
	//bool isBomber;        	 // �÷��̾���� ����
	//XMFLOAT3 Pos; 	             // �÷��̾� ��ġ
	//XMFLOAT4 Dir;      	// ����(���ʹϾ�)
	//byte AnimationNum;    	// �ִϸ��̼� ��ȣ
	//float AnimationTime; 	// �ִϸ��̼� �ð� ����
	//byte UsedItem;      	// ���Ǵ� ������ ����
	//byte PlayerState;  	// �÷��̾� ����
};


//////////////////////////////////////////////////////
