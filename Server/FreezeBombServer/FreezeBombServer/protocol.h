#pragma once

#define SERVER_IP "172.30.1.1"

using namespace std;

constexpr int MAX_USER = 6;
constexpr int MAX_ROUND = 3;

struct clientsInfo
{
	char    id;
	bool	isReady;
	char	name[32];
};

enum ROLE { RUNNER, BOMBER };
enum ITEM { NORMALHAMMER=0, GOLD_HAMMER, GOLD_TIMER,EMPTY };

enum PLAYER_NUM { P1, P2, P3, P4, P5, P6 };						// ��� �÷��̾� ���� 
enum PLAYER_STATE { NONESTATE, ICE, BREAK };							// �÷��̾� ����
enum STATE_TYPE { Init, Run, Over };
enum MATERIAL { PINK, BROWN, WHITE, BLACK, BLUE, PANDA, ICEMAT };

constexpr int MAX_ITEM_NAME_LENGTH = 16;
constexpr int MAX_CHATTING_LENGTH = 100;
constexpr int COOLTIME = 3;

constexpr int SC_ACCESS_COMPLETE = 1;
constexpr int SC_PUT_PLAYER = 2;
constexpr int SC_MOVE_PLAYER = 3;
constexpr int SC_REMOVE_PLAYER = 4;
constexpr int SC_USE_ITEM = 5;
constexpr int SC_ROLE_CHANGE = 6;
constexpr int SC_ROUND_END = 7;
constexpr int SC_ROUND_START = 8;
constexpr int SC_PLEASE_READY = 9;
constexpr int SC_ACCESS_PLAYER = 10;
constexpr int SC_COMPARE_TIME = 11;
constexpr int SC_STOP_RUN_ANIM = 12;
constexpr int SC_ANIMATION_INFO = 13;
constexpr int SC_CLIENT_LOBBY_IN = 14;
constexpr int SC_CLIENT_LOBBY_OUT = 15;
constexpr int SC_CHATTING = 16;
constexpr int SC_READY_STATE = 17;
constexpr int SC_UNREADY_STATE = 18;
constexpr int SC_FREEZE = 19;
constexpr int SC_RELEASE_FREEZE = 20;
constexpr int SC_BOMB_EXPLOSION = 21;
constexpr int SC_CHANGE_HOST_ID = 22;
constexpr int SC_GET_ITEM = 23;
constexpr int SC_ROUND_SCORE = 24;
constexpr int SC_CHOICE_CHARACTER = 25;
constexpr int SC_CHOSEN_CHARACTER = 26;

constexpr int CS_UP_KEY = 0;
constexpr int CS_DOWN_KEY = 1;
constexpr int CS_RIGHT_KEY = 2;
constexpr int CS_LEFT_KEY = 3;
constexpr int CS_UPLEFT_KEY = 4;
constexpr int CS_UPRIGHT_KEY = 5;
constexpr int CS_DOWNLEFT_KEY = 6;
constexpr int CS_DOWNRIGHT_KEY = 7;
constexpr int CS_READY = 8;
constexpr int CS_UNREADY = 9;
constexpr int CS_REQUEST_START = 10;
constexpr int CS_RELEASE_KEY = 11;
constexpr int CS_ANIMATION_INFO = 12;
constexpr int CS_NICKNAME_INFO = 13;
constexpr int CS_CHATTING = 14;
constexpr int CS_OBJECT_COLLISION = 15;
constexpr int CS_NOT_COLLISION = 16;
constexpr int CS_PLAYER_COLLISION = 17;
constexpr int CS_NOT_PLAYER_COLLISION = 18;
constexpr int CS_USEITEM = 19;
constexpr int CS_FREEZE = 20;
constexpr int CS_RELEASE_FREEZE = 21;
constexpr int CS_BOMB_EXPLOSION = 22;
constexpr int CS_BOMBER_TOUCH = 23;
constexpr int CS_GET_ITEM = 24;
constexpr int CS_CHOICE_CHARACTER = 25;


constexpr int MAX_ROUND_TIME = 100;

//[Ŭ��->����]



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

struct CS_PACKET_FREEZE					//�������� �˸� ��Ŷ
{
	char size;
	char type;
};

struct CS_PACKET_RELEASE_FREEZE		//���� ���� �˸� ��Ŷ
{
	char size;
	char type;
};

struct CS_PACKET_READY
{
	char size;
	char type;
};

struct CS_PACKET_UNREADY
{
	char size;
	char type;
};

struct CS_PACKET_REQUEST_START
{
	char size;
	char type;
};

struct CS_PACKET_ANIMATION
{
	char size;
	char type;
	char animation;			//�ִϸ��̼� ������ Ŭ�󿡼� �޾ƿ��� ��Ŷ
	char padding;			//4����Ʈ ������ ���� 
	//float animationTime;	//���� �ִϸ��̼� �ð�
};

struct CS_PACKET_BOMBER_TOUCH
{
	char size;
	char type;
	char touchId;	// ��ġ�� �÷��̾� ��ȣ
};

struct CS_PACKET_RELEASE_KEY
{
	char size;
	char type;
};

// �÷��̾� �г��� ������ �뺸
struct CS_PACKET_NICKNAME
{
	char size;
	char type;
	char id;
	char padding;	//4����Ʈ ���������� 
	char name[24];
};

struct CS_PACKET_CHATTING
{
	char size;
	char type;
	char id;
	char padding;
	char chatting[MAX_CHATTING_LENGTH];
};

struct CS_PACKET_OBJECT_COLLISION
{
	char size;
	char type;
	unsigned short objId;		//object������ 66536�� ���� �ʱ� ������ unsigned short�� ����
};

struct CS_PACKET_NOT_COLLISION
{
	char size;
	char type;
};
struct CS_PACKET_PLAYER_COLLISION
{
	char size;
	char type;
	unsigned char playerID;
};

struct CS_PACKET_GET_ITEM
{
	char size;
	char type;
	char itemIndex[MAX_ITEM_NAME_LENGTH];
};

struct CS_PACKET_USE_ITEM
{
	char size;
	char type;
	char target;			// ������� ������ ��
	char usedItem;			// ���Ǵ� ������ ����
};

struct CS_PACKET_BOMB_EXPLOSION
{
	char size;
	char type;
};

struct CS_PACKET_CHOICE_CHARACTER
{
	char size;
	char type;
	char matID;
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

//<< Ready Room ��Ŷ ���� >>
struct SC_PACKET_ACCESS_COMPLETE
{
	char size;
	char type;
	char myId;
	char hostId;
	char score;				// �÷��̾� ����
	char roundCount;		// �� ��������
	char serverTime;				// ���� �ð�
};

struct SC_PACKET_ACCESS_PLAYER
{
	char size;
	char type;
	char id;
};

struct SC_PACKET_CHANGE_HOST
{
	char size;
	char type;
	char hostID;
};
//������ Ŭ���̾�Ʈ�� ����
struct SC_PACKET_LOBBY_IN
{
	char size;
	char type;
	char id;
	clientsInfo client_state;
};

//������ Ŭ���̾�Ʈ�� ����
struct SC_PACKET_LOBBY_OUT
{
	char size;
	char type;
	char id;
};

struct SC_PACKET_PLEASE_READY
{
	char size;
	char type;
};

struct SC_PACKET_ROUND_START
{
	char size;
	char type;
	char clientCount;
	char bomberID;
	char goldTimerCnt;
	char goldHammerCnt;
	char hammerCnt;
	char round;
	unsigned short startTime;
};

struct SC_PACKET_PUT_PLAYER
{
	//4����Ʈ ������ �� �ʿ䰡 �ִ�.
	char size;
	char type;
	char posIdx[MAX_USER];
};

// �÷��̾� �̵� ��
struct SC_PACKET_MOVE_PLAYER
{
	
	char size;
	char type;
	char id;
	char padding;		//4����Ʈ ����
	float xPos;
	float yPos;
	float zPos;

	//ĳ������ ���� ����
	float xLook;
	float yLook;
	float zLook;
	float xUp;
	float yUp;
	float zUp;
	float xRight;
	float yRight;
	float zRight;

	//�� ��ü�� ȸ�� 
	float pitch;
	float yaw;
	float roll;	

	//�ӵ�
	float fVelocity;

	bool  isMoveRotate;
};

struct SC_PACKET_PLAYER_ANIMATION
{
	char size;
	char type;
	char id;
	char animation;
	
};

struct SC_PACKET_STOP_RUN_ANIM
{
	char size;
	char type;
	char id;
};
//���� Ready���� �÷��̾��� ������ ���� ��Ŷ
struct SC_PACKET_READY_STATE
{
	char size;
	char type;
	char id;
};

struct SC_PACKET_UNREADY_STATE
{
	char size;
	char type;
	char id;
};

struct SC_PACKET_CHATTING
{
	char size;
	char type;
	char id;
	char padding;
	char message[MAX_CHATTING_LENGTH];
};
// �÷��̾ ������ ��� ��
struct SC_PACKET_GET_ITEM
{
	char size;
	char type;
	char id;
	char itemIndex[MAX_ITEM_NAME_LENGTH];
};

struct SC_PACKET_USE_ITEM
{
	char size;
	char type;
	char id;
	char target;
	char usedItem;			// ���Ǵ� ������ ����
};

// ���� �������� �����ð��� Ŭ��ð��� ���ϱ� ����
// �ٸ� ��� Ŭ��ð��� �����ð����� �缳��
struct SC_PACKET_COMPARE_TIME
{
	char size;
	char type;
	unsigned short serverTime;				// ���� �ð�
};

struct SC_PACKET_REMOVE_PLAYER
{
	char size;
	char type;
	char id;
	char hostId;
};

struct SC_PACKET_ROUND_END
{
	char size;
	char type;
	bool isWinner;
};

struct SC_PACKET_COLLIDED
{
	char size;
	char type;
	char id;
};

struct SC_PACKET_NOT_COLLIDED
{
	char size;
	char type;
	char id;
};

struct SC_PACKET_FREEZE	
{
	char size;
	char type;
	char id;
};

struct SC_PACKET_RELEASE_FREEZE
{
	char size;
	char type;
	char id;
};

struct SC_PACKET_BOMB_EXPLOSION
{
	char size;
	char type;
	char bomberId;			
};

struct SC_PACKET_ROLE_CHANGE
{
	char size;
	char type;
	char bomberId;
	char runnerId;
};

struct SC_PACKET_ROUND_SCORE
{
	char size;
	char type;
	char score[MAX_USER];
};

struct SC_PACKET_CHOICE_CHARACTER
{
	char size;
	char type;
	char id;
	char matID;
};

struct SC_PACKET_CHOSEN_CHARACTER
{
	char size;
	char type;
	char matID[MAX_USER];
};
//////////////////////////////////////////////////////
