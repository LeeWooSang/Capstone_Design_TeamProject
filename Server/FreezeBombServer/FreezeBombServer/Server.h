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

constexpr int MAX_WORKER_THREAD = 2;

// Overlapped����ü Ȯ��
struct OVER_EX {
	WSAOVERLAPPED	over;
	WSABUF			dataBuffer;
	char			messageBuffer[MAX_BUFFER];
	bool			is_recv;
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
	float maxVelocityXZ;
	float maxVelocityY;

	XMFLOAT3 lastRightVector;
	XMFLOAT3 lastLookVector;
	XMFLOAT3 lastUpVector;

	
	char score;
	char normalItem;
	char specialItem;
	char role;
	char matID;

	char nickname[32];
	//wchar_t nickname[12];
	char animation;			//�ִϸ��̼� index
	float animationTime;	//�ִϸ��̼� �ð�
	bool isReady;
public:
	SOCKETINFO() {
		in_use = false;
		score = 0;
		normalItem = ITEM::NONEITEM;
		specialItem = ITEM::NONEITEM;
		role = ROLE::RUNNER;
		isReady = false;
		maxVelocityXZ = 300.0f;
		maxVelocityY = 400.0f;
		ZeroMemory(nickname, sizeof(wchar_t) * 12);
		ZeroMemory(&over_ex.messageBuffer, sizeof(over_ex.messageBuffer));
		ZeroMemory(&packet_buffer, sizeof(packet_buffer));
		over_ex.dataBuffer.len = MAX_BUFFER;
		over_ex.dataBuffer.buf = over_ex.messageBuffer;
		over_ex.is_recv = true;
	}
};

class MAPOBJECT
{
public:
	XMFLOAT3 pos;
};

class Server
{
private:
	mutex gLock;
	mutex clientsLock[MAX_USER];
	SOCKET listenSocket;
	HANDLE iocp;
	// vector�� ���� �� over_ex.messagebuffer�� ���� ������ �ʴ´�.
	// �迭�� �ٲٴ� ����� ������. ��? ���� ����?
	SOCKETINFO clients[MAX_USER];
	vector<thread> workerThreads;
	CGameTimer gameTimer;
	CHeightMapImage* heightMap;
	XMFLOAT3 gravity;
	vector<MAPOBJECT> objects;

	float roundStartTime;
	float roundCurrTime;
	int clientCount;
	int readyCount;
	int hostId;
	int bomberID;
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
	void SendClientLobbyIn(char toClient, char fromClient,char *name);
	void SendClientLobbyOut(char toClient, char fromClient);
	void SendChattinPacket(char to, char from, char *message);
	void SendPlayerAnimation(char toClient, char fromCllient);
	void SendPutPlayer(char toClient, char fromClient);
	void SendRoundStart(char client);
	void SendPleaseReady(char client);
	void SendReadyStatePacket(char toClient, char fromClient);
	void SendUnReadyStatePacket(char toClient, char fromClient);
	void SendMovePlayer(char to,char object);
	void SendRemovePlayer(char toClient, char fromClient);
	void SendRoundEnd(char client);
	void SendCompareTime(char client);
	void SendStopRunAnim(char toClient, char fromClient);
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
public:
	void PickBomber();
	void StartTimer();
	void ResetTimer();
	void LoadMapObjectInfo();
public:
	bool InitServer();
	void RunServer();
public:
	void err_quit(const char*);
	void err_display(const char*);
};

