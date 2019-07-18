#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "MyInclude.h"
#include "MyDefine.h"
#include "protocol.h"
#include "Terrain.h"
#include "GameTimer.h"

#pragma comment(lib, "Ws2_32.lib")
constexpr int MAX_BUFFER = 1024;

constexpr int SERVER_PORT = 9000;

constexpr float PLAYER_INIT_X_POS = 40;
constexpr float PLAYER_INIT_Y_POS = 0;
constexpr float PLAYER_INIT_Z_POS = 40;

constexpr float PLAYER_INIT_X_LOOK = 0;
constexpr float PLAYER_INIT_Y_LOOK = 0;
constexpr float PLAYER_INIT_Z_LOOK = 0;

constexpr float PLAYER_INIT_X_UP = 0;
constexpr float PLAYER_INIT_Y_UP = 0;
constexpr float PLAYER_INIT_Z_UP = 0;

constexpr float PLAYER_INIT_X_RIGHT = 0;
constexpr float PLAYER_INIT_Y_RIGHT = 0;
constexpr float PLAYER_INIT_Z_RIGHT = 0;

constexpr float FRICTION = 250;
constexpr float MAX_VELOCITY_XZ = 40;
constexpr float MAX_VELOCITY_Y = 400;
constexpr float VELOCITY = 0.7f;

constexpr int MAX_FREEZE_COUNT = MAX_USER - 2;
constexpr int MAX_WORKER_THREAD = 2;

constexpr int GOLD_HAMMER_COUNT = 2;

enum EVENT_TYPE
{
	EV_RECV,
	EV_SEND,
	EV_COUNT,
	EV_COOLTIME,
	EV_NEXTROUNDSTART
};
enum COLLISION_TYPE		//��� ��ü�� �浹�ߴ��� 
{
	CL_NONE,
	CL_SURROUNDING,	//�ֺ�
	CL_PLAYER		//�÷��̾�
};

enum GAME_STATE			//�κ� �������� �ΰ��� ������ 
{
	GS_LOBBY,
	GS_INGAME
};
struct EVENT_ST
{
	int					obj_id;
	EVENT_TYPE			type;
	high_resolution_clock::time_point start_time;
	constexpr bool operator<(const EVENT_ST& left)const
	{
		return(type > left.type && start_time > left.start_time);
	}
};

// Overlapped����ü Ȯ��
struct OVER_EX {
	WSAOVERLAPPED	over;
	WSABUF			dataBuffer;
	char			messageBuffer[MAX_BUFFER];
	EVENT_TYPE		event_t;
};

class SOCKETINFO
{
public:
	// in_use�� x,y���� data race�߻�
	// in_use�� true�ΰ� Ȯ���ϰ� send�Ϸ��� �ϴµ� �� ������ ���� �����ؼ� false���ȴٸ�?
	// �׸��� send ���� �� �÷��̾ id�� �����Ѵٸ�? => ������ ���� send�� ��
	// mutex access_lock;
	bool in_use;
	OVER_EX over_ex;
	SOCKET socket;
	// �����Ұ��� �޸𸮸� �������� �����ϱ� ���� �ӽ������
	char packet_buffer[MAX_BUFFER];
	int prev_size;

	XMFLOAT3 pos;
	XMFLOAT3 look;
	XMFLOAT3 up;
	XMFLOAT3 right;
	XMFLOAT3 velocity;
	float pitch;
	float yaw;
	float roll;
	DWORD direction;
	//�ӵ�
	float fVelocity;

	XMFLOAT3 lastRightVector;
	XMFLOAT3 lastLookVector;
	XMFLOAT3 lastUpVector;

	
	COLLISION_TYPE collision;
	char score;
	char normalItem;
	char specialItem;
	char role;
	char matID;
	bool isFreeze;			//���� �������� ����
	bool isMoveRotate;		//�ִϸ��̼��� �ݺ��ؼ� �����ϴ� ������ �ذ��ϱ� ���� ����
	char nickname[32];
	//wchar_t nickname[12];
	char animation;			//�ִϸ��̼� index
	float animationTime;	//�ִϸ��̼� �ð�
	bool isReady;
	GAME_STATE gameState;	

public:
	SOCKETINFO() {
		in_use = false;
		score = 0;
		normalItem = ITEM::EMPTY;
		specialItem = ITEM::EMPTY;
		role = ROLE::RUNNER;
		matID = 0;
		isReady = false;
		ZeroMemory(nickname, sizeof(wchar_t) * 12);
		ZeroMemory(&over_ex.messageBuffer, sizeof(over_ex.messageBuffer));
		ZeroMemory(&packet_buffer, sizeof(packet_buffer));
		over_ex.dataBuffer.len = MAX_BUFFER;
		over_ex.dataBuffer.buf = over_ex.messageBuffer;
		over_ex.event_t = EV_RECV;
	}
};

struct MAPOBJECT
{
	string name;
	XMFLOAT3 pos = XMFLOAT3(0.0f,0.0f,0.0f);
};

class Server
{
private:
	int round;
	mutex gLock;
	SOCKET listenSocket;
	HANDLE iocp;
	// vector�� ���� �� over_ex.messagebuffer�� ���� ������ �ʴ´�.
	// �迭�� �ٲٴ� ����� ������. ��? ���� ����?
	SOCKETINFO clients[MAX_USER];
	vector<thread> workerThreads;
	CGameTimer gameTimer;
	CHeightMapImage* heightMap;
	XMFLOAT3 gravity;
	unsigned short roundCurrTime;
	short changeCoolTime;	//Bomber�� �浹������ ������� �ð���ŭ�� ������ �� 
	int clientCount;
	int readyCount;
	int hostId;
	int bomberID;
	int freezeCnt;			//���� ������ ������ ��  
	
	int goldTimerCnt[MAX_ROUND];
	int goldHammerCnt[MAX_ROUND];
	int normalHammerCnt[MAX_ROUND];

	int randomPosIdx[MAX_USER];

private:

	priority_queue <EVENT_ST> timer_queue;
	mutex		timer_l;
	mutex		roundTime_l;
	mutex		coolTime_l;
private:
	vector<MAPOBJECT> objects[MAX_ROUND];
	vector<MAPOBJECT> goldTimers[MAX_ROUND];
	vector<MAPOBJECT> goldHammers[MAX_ROUND];
	vector<MAPOBJECT> NormalHammers[MAX_ROUND];
	MAPOBJECT recent_objects;		//�ֱٿ� �ε��� ������Ʈ;
	XMFLOAT3 recent_pos;
	XMFLOAT3 player_pos;
	unsigned char recent_player;
public:
	Server();
	~Server();
public:
	static void AcceptThread(LPVOID arg);
	void AcceptThreadFunc();
	static void WorkerThread(LPVOID arg);
	void WorkerThreadFunc();
	static void TimerThread(LPVOID arg);
	void TimerThreadFunc();
public:
	void ProcessPacket(char client, char *packet);
	void SendFunc(char client, void *packet);
	void RecvFunc(char client);
	void ClientDisconnect(char client);
public:
	void SendAccessComplete(char client);
	void SendAccessPlayer(char toClient, char fromClient);
	void SendClientLobbyIn(char toClient, char fromClient,char *name,const bool& isReady);
	void SendClientLobbyOut(char toClient, char fromClient);
	void SendChattinPacket(char to, char from, char *message);
	void SendPlayerAnimation(char toClient, char fromCllient);
	void SendPutPlayer(char toClient);
	void SendRoundStart(char client,unsigned short& time);
	void SendPleaseReady(char client);
	void SendReadyStatePacket(char toClient, char fromClient);
	void SendUnReadyStatePacket(char toClient, char fromClient);
	void SendMovePlayer(char to,char object);
	void SendRemovePlayer(char toClient, char fromClient);
	void SendRoundEnd(char client);
	void SendCompareTime(char client,unsigned short& time);
	void SendStopRunAnim(char toClient, char fromClient);
	void SendUseItem(char toClient, char fromClient, char useItem, char targetClient);
	void SendFreeze(char toClient, char fromClient);
	void SendReleaseFreeze(char toClient, char fromClient);
	void SendBombExplosion(char toClient, char fromClient);
	void SendChangeBomber(char toClient, char bomberId,char runnerId);
	void SendChangeHostID(char toClient, char hostID);
	void SendGetItem(char toClient, char fromClient, string& itemIndex);
	void SendRoundScore(char client);
	void SendChoiceCharacter(char toClient, char fromClient, char matID);
	void SendChosenCharacter(char toClient);
public:
	void SetAnimationState(char client,char animationNum);
	void SetVelocityZero(char client);
	void SetPitchYawRollZero(char client);
	void SetClient_Initialize(char client);
	void SetDirection(char client, int key);
	void RotateModel(char client, float x, float y, float z);
	void RotateClientAxisY(char client, float fTimeElapsed);
	void UpdateClientPos(char client, float fTimeElapsed);
	void ProcessClientHeight(char client);
	void ProcessFriction(char client, float& fLength);
	void ShuffleStartPosIndex();
public:
	void InitGame();
	void InitRound();
	void PickBomber();
	void StartTimer();
	void ResetTimer();

	void add_timer(int obj_id,EVENT_TYPE et,chrono::high_resolution_clock::time_point start_time);
public:
	bool InitServer();
	void RunServer();

public:
	void LoadMapObjectInfo();
public:
	void err_quit(const char*);
	void err_display(const char*);
};

