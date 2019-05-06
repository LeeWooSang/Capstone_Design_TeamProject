#pragma once
//#include <iostream>
//#include <windows.h>
//#include <DirectXMath.h>

using namespace std;
//using namespace DirectX;

enum ROLE { RUNNER, BOMBER };
enum ITEM { NONEITEM = 0, HAMMER, GOLD_HAMMER, GOLD_TIMER, BOMB };
enum PLAYER_NUM { P1, P2, P3, P4, P5, P6 };						// ��� �÷��̾� ���� 
enum PLAYER_STATE { NONESTATE, ICE, BREAK };							// �÷��̾� ����
enum STATE_TYPE { Init, Run, Over };
//�� Ŭ���̾�Ʈ���� ������ ĳ���� ������ �׷������ϱ� ������
//������ MATERIAL������ ������ �� Ŭ���̾�Ʈ�鿡�� PUT_PLAYER��Ŷ�� �����鼭 
// �˷���� �Ѵ�. -> �׷��� Ŭ���̾�Ʈ�� �� ������ �޾� Evilbear���� �ش� ������ �ʱ�ȭ ���� �� ����
//EX> 1�� Ŭ���̾�Ʈ�� PINK - 2�� Ŭ���̾�Ʈ�� PANDA 
enum MATERIAL{PINK, BROWN, WHITE, BLACK, BLUE, PANDA, ICEMAT};


constexpr int SC_ACCESS_COMPLETE = 1;
constexpr int SC_PUT_PLAYER = 2;
constexpr int SC_MOVE_PLAYER = 3;
constexpr int SC_REMOVE_PLAYER = 4;
constexpr int SC_USE_ITEM = 5;
constexpr int SC_ROLL_CHANGE = 6;
constexpr int SC_ROUND_END = 7;

constexpr int CS_UP_KEY = 0;
constexpr int CS_DOWN_KEY = 1;
constexpr int CS_RIGHT_KEY = 2;
constexpr int CS_LEFT_KEY = 3;
constexpr int CS_CTRL_KEY = 4;

//[Ŭ��->����]

struct SC_PACKET_ACCESS_COMPLETE
{
	char size;
	char type;
	char myId;
	char score;				// �÷��̾� ����
	char roundCount;		// �� ��������
	char serverTime;				// ���� �ð�
};

struct SC_PACKET_PUT_PLAYER
{
	char size;
	char type;
	char myId;
	char score;		// �÷��̾� ����
	char matID;			//������ ���ϴ� ĳ���ʹ� ���������� �ʿ��ϴ�.
	char xPos;		// ������Ʈ�� ��ġ
	char yPos;
	char zPos;
	//�̷��� �ٲ��� �ɵ�
	char xLook;
	char yLook;
	char zLook;
	char xUp;
	char yUp;
	char zUp;
	char xRight;
	char yRight;
	char zRight;
};

//////////////////////////////////////////////////////

//<< InGame ��Ŷ ���� >>

//[Ŭ��->����]
struct CS_PACKET_RIGHT_KEY
{
	char size;
	char type;
};

struct CS_PACKET_LEFT_KEY
{
	char size;
	char type;
};

struct CS_PACKET_UP_KEY
{
	char size;
	char type;
};

struct CS_PACKET_DOWN_KEY
{
	char size;
	char type;
};

struct CS_PACKET_BOMBER_TOUCH
{
	char size;
	char type;
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
struct SC_PACKET_INGAME_PACKET
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
struct SC_PACKET_MOVE_PLAYER
{
	char size;
	char type;
	char id;
	char xPos;
	char yPos;
	char zPos;
	char xLook;
	char yLook;
	char zLook;
	char xUp;
	char yUp;
	char zUp;
	char xRight;
	char yRight;
	char zRight;
};

// �÷��̾ ������ ��� ��
struct SC_PACKET_USE_ITEM
{
	char size;
	char type;
	char id;
	char usedItem;			// ���Ǵ� ������ ����
};

// ���� �������� �����ð��� Ŭ��ð��� ���ϱ� ����
// �ٸ� ��� Ŭ��ð��� �����ð����� �缳��
struct SC_PACKET_COMPARE_TIME
{
	char size;
	char type;
	char serverTime;				// ���� �ð�
};

struct SC_PACKET_ROLL_CHANGE
{
	char size;
	char type;
	char bomberId;
	char normalId;
};

struct SC_PACKET_REMOVE_PLAYER
{
	char size;
	char type;
	char id;
};

struct SC_PACKET_ROUND_END
{
	char size;
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
