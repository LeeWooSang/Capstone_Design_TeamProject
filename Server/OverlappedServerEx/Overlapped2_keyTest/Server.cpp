#pragma comment(lib, "ws2_32")
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>

#define SERVERPORT 9000
#define BUFSIZE 512

const static int UP_KEY = 72;
const static int DOWN_KEY = 80;
const static int RIGHT_KEY = 77;
const static int LEFT_KEY = 75;

const static int PLAYER_1 = 0;
const static int PLAYER_2 = 1;
const static int PLAYER_3 = 2;
const static int PLAYER_4 = 3;
const static int PLAYER_5 = 4;
const static int PLAYER_6 = 5;


struct POSITION
{
	POSITION() :x(0), y(0), z(0) {};
	POSITION(int x1, int y1, int z1) :x(x1), y(y1), z(z1) {};
	int x, y, z;
};

class ClientInfo
{
private:
	u_short clientID;
	POSITION pos;
	int key;

public:
	ClientInfo() :clientID(0), pos(0, 0, 0), key(0) {};
	ClientInfo(u_short cid, POSITION p) :clientID(cid), pos(p), key(0) {};
	
	u_short getClientID() { return clientID; }
	void getPos(int& x, int& y, int& z) { x = pos.x; y = pos.y; z = pos.z; }
	int getKey() { return key; }

	void setClientID(u_short cid) { clientID = cid; }
	void setPos(int x, int y, int z) { pos.x = x; pos.y = y; pos.z = z; }
	void setPos(POSITION* p) { pos = *p; }
	void setKey(int k) { key = k; }

public:
	// ������԰� ������� ����
	ClientInfo(const ClientInfo&) = delete;
	ClientInfo& operator=(const ClientInfo&) = delete;
};

// ���� ���� ������ ���� ����ü�� ����
struct SOCKETINFO
{
	SOCKETINFO() { cInfo = new ClientInfo; }
	~SOCKETINFO() { delete cInfo; }
	WSAOVERLAPPED overlapped;
	SOCKET sock;
	ClientInfo* cInfo;
	int recvbytes;
	int sendbytes;
	WSABUF wsabuf;
};


// accept()�� ���ϰ��� ������ ����. �� �����忡�� �����ϹǷ� ���� ������ ����.
SOCKET client_sock;
// client_sock ������ ��ȣ�ϱ� ���� �̺�Ʈ ��ü �ڵ�.
HANDLE hReadEvent, hWriteEvent;

std::vector<SOCKETINFO> vClient;
int clientCount;
CRITICAL_SECTION cs;

// �񵿱� ����� ���۰� ó�� �Լ�
DWORD WINAPI WorkerThread(LPVOID arg);
void CALLBACK RecvCompletionRoutine(DWORD dwError, DWORD cbTransferred, LPWSAOVERLAPPED lpOverlapped, DWORD dwFlags);
void CALLBACK SendCompletionRoutine(DWORD dwError, DWORD cbTransferred, LPWSAOVERLAPPED lpOverlapped, DWORD dwFlags);
// ���� ��� �Լ�
void err_quit(const char *msg);
void err_display(const char *msg);
void err_display(const int errcode);

int main(int argc, char *argv[])
{
	InitializeCriticalSection(&cs);
	int retval;
	vClient.reserve(6);
	// ���� �ʱ�ȭ
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// socket()
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET)
		err_quit("socket()");

	// bind()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = bind(listen_sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR)
		err_quit("bind()");

	// listen()
	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR)
		err_quit("listen()");

	// �̺�Ʈ ��ü ����
	// CreateEvent(����, true->�������� / false->�ڵ�����, true->��ȣ�ν��� / false->���ȣ�ν���, �̺�Ʈ�̸�)
	// WorkerThread�� client_sock�� ���� �о����� main �����忡 �˸� (�б� �˸�)
	hReadEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
	if (hReadEvent == NULL)
		return 1;
	// main �����尡 client_sock�� ���� ���������� alertable wait ������ WorkerThread�� �˸� (���� �˸�)
	hWriteEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (hWriteEvent == NULL)
		return 1;

	// ������ ����
	HANDLE hThread = CreateThread(NULL, 0, WorkerThread, NULL, 0, NULL);
	if (hThread == NULL)
		return 1;
	CloseHandle(hThread);

	while (1)
	{
		WaitForSingleObject(hReadEvent, INFINITE);
		// accept()
		client_sock = accept(listen_sock, NULL, NULL);
		if (client_sock == INVALID_SOCKET)
		{
			err_display("accept()");
			break;
		}
		SetEvent(hWriteEvent);
	}

	DeleteCriticalSection(&cs);
	// ���� ����
	WSACleanup();
	return 0;
}

// �񵿱� ����� ó�� �Լ�
DWORD WINAPI WorkerThread(LPVOID arg)
{
	int retval;

	while (1)
	{
		while (1)
		{
			// alertable wait
			DWORD result = WaitForSingleObjectEx(hWriteEvent, INFINITE, TRUE);
			// ���ο� Ŭ���̾�Ʈ�� ������ ��� -> ������ �����.
			if (result == WAIT_OBJECT_0)
				break;
			// WAIT_IO_COMPLETION: (�񵿱� ����� �۾� -> )�Ϸ� ��ƾ ȣ�� �Ϸ� -> �ٽ� alertable wait ���¿� ����.
			if (result != WAIT_IO_COMPLETION)
				return 1;
		}

		// ������ Ŭ���̾�Ʈ ���� ���
		SOCKADDR_IN clientaddr;
		int addrlen = sizeof(clientaddr);
		getpeername(client_sock, (SOCKADDR *)&clientaddr, &addrlen);
		printf("\n[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ� = %s, ��Ʈ ��ȣ = %d\n",
			inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

		// ���� ���� ����ü �Ҵ�� �ʱ�ȭ
		SOCKETINFO *ptr = new SOCKETINFO;
		ptr->cInfo = new ClientInfo((u_short)clientCount, POSITION(0, 0, 0));
		clientCount++;

		if (ptr == NULL)
		{
			printf("[����] �޸𸮰� �����մϴ�!\n");
			return 1;
		}
		ZeroMemory(&ptr->overlapped, sizeof(ptr->overlapped));
		ptr->sock = client_sock;
		// client_sock ���� ���� �о�� ��� hReadEent ��ȣ ���·� ��ȯ
		SetEvent(hReadEvent);
		ptr->recvbytes = ptr->sendbytes = 0;
		// ������???? cInfo�� �����Ҵ��� ���������?
		// ���μ������� ������ �����ϰ� �ش� ��ü �����͸� �Ѱ��ٱ�?? 
		ptr->wsabuf.buf = (char *)ptr->cInfo;
		ptr->wsabuf.len = sizeof(ClientInfo);
		// ���� �޾ƿ����� client���� ������ ��� ���Ϳ� ����
		vClient.emplace_back(*ptr);

		// �񵿱� ����� ����
		DWORD recvbytes;
		DWORD flags = 0;
		retval = WSARecv(ptr->sock, &ptr->wsabuf, 1, &recvbytes, &flags, &ptr->overlapped, RecvCompletionRoutine);
		if (retval == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSA_IO_PENDING)
			{
				err_display("WSARecv()");
				return 1;
			}
		}

		//DWORD sendbytes;
		//retval = WSASend(ptr->sock, &ptr->wsabuf, 1, &sendbytes, flags, &ptr->overlapped, RecvCompletionRoutine);
		//if (retval == SOCKET_ERROR)
		//{
		//	if (WSAGetLastError() != WSA_IO_PENDING)
		//	{
		//		err_display("WSARecv()");
		//		return 1;
		//	}
		//}
		// delete ptr;
	}
	return 0;
}

// RECV �񵿱� ����� ó�� �Լ�(����� �Ϸ� ��ƾ)
void CALLBACK RecvCompletionRoutine(DWORD dwError, DWORD cbTransferred, LPWSAOVERLAPPED lpOverlapped, DWORD dwFlags)
{
	int retval;

	// Ŭ���̾�Ʈ ���� ���
	SOCKETINFO *ptr = (SOCKETINFO *)lpOverlapped;
	SOCKADDR_IN clientaddr;
	int addrlen = sizeof(clientaddr);
	getpeername(ptr->sock, (SOCKADDR *)&clientaddr, &addrlen);

	// �񵿱� ����� ��� Ȯ��
	if (dwError != 0 || cbTransferred == 0)
	{
		if (dwError != 0)
			err_display(dwError);
		closesocket(ptr->sock);
		printf("[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ� = %s, ��Ʈ ��ȣ = %d\n",
			inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
		delete ptr;
		return;
	}

	// recv ��ó�� �ڵ�
	// ���� key�� position�� ������ ����
	int key = 0;
	POSITION tmpPos;

	key = ptr->cInfo->getKey();
	ptr->cInfo->getPos(tmpPos.x, tmpPos.y, tmpPos.z);
	// �� key���� 0�� ����?
	printf("%d\n", key);

	switch (key)
	{
	case UP_KEY:
		printf("UP_KEY ����\n");
		break;
	case DOWN_KEY:
		printf("DOWN_KEY ����\n");
		break;
	case RIGHT_KEY:
		printf("RIGHT_KEY ����\n");
		break;
	case LEFT_KEY:
		printf("LEFT_KEY ����\n");
		break;
	}
	// ������ �ޱ�
	ZeroMemory(&ptr->overlapped, sizeof(ptr->overlapped));
	ptr->wsabuf.buf = (char *)ptr->cInfo;
	ptr->wsabuf.len = sizeof(ClientInfo);
	
	DWORD recvbytes;
	DWORD flags = 0;
	retval = WSARecv(ptr->sock, &ptr->wsabuf, 1, &recvbytes, &flags, &ptr->overlapped, RecvCompletionRoutine);
	if (retval == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			err_display("WSARecv()");
			return;
		}
	}
	//EnterCriticalSection(&cs);
	//vClient[ptr->cInfo->getClientID()].cInfo->setKey(key);
	//vClient[ptr->cInfo->getClientID()].cInfo->setPos(&tmpPos);
	//LeaveCriticalSection(&cs);
	//
	//// ����� ������ send
	//DWORD sendbytes;
	//retval = WSASend(ptr->sock, &ptr->wsabuf, 1, &sendbytes, 0, &ptr->overlapped, SendCompletionRoutine);
	//if (retval == SOCKET_ERROR)
	//{
	//	if (WSAGetLastError() != WSA_IO_PENDING)
	//	{
	//		err_display("WSASend()");
	//		return;
	//	}
	//}
}

// SEND �񵿱� ����� ó�� �Լ�(����� �Ϸ� ��ƾ)
void CALLBACK SendCompletionRoutine(DWORD dwError, DWORD cbTransferred, LPWSAOVERLAPPED lpOverlapped, DWORD dwFlags)
{
	int retval;

	// Ŭ���̾�Ʈ ���� ���
	SOCKETINFO *ptr = (SOCKETINFO *)lpOverlapped;
	SOCKADDR_IN clientaddr;
	int addrlen = sizeof(clientaddr);
	getpeername(ptr->sock, (SOCKADDR *)&clientaddr, &addrlen);

	// �񵿱� ����� ��� Ȯ��
	if (dwError != 0 || cbTransferred == 0)
	{
		if (dwError != 0)
			err_display(dwError);
		closesocket(ptr->sock);
		printf("[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ� = %s, ��Ʈ ��ȣ = %d\n",
			inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
		delete ptr;
		return;
	}

	// send ��ó�� �ڵ�

}

// ���� �Լ� ���� ��� �� ����
void err_quit(const char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);
}

// ���� �Լ� ���� ���
void err_display(const char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	printf("[%s] %s", msg, (char *)lpMsgBuf);
	LocalFree(lpMsgBuf);
}

void err_display(const int errcode)
{
	LPVOID lpMsgBuf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, errcode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&lpMsgBuf, 0, NULL);
	printf("[����] %s", (char *)lpMsgBuf);
	LocalFree(lpMsgBuf);
}