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

// Ŭ���̾�Ʈ���� ������ ��Ƴ��� ������ ��ü
class ClientInfo
{
private:
	POSITION pos;
	SOCKET sock;

public:
	ClientInfo() :pos(0, 0, 0){};
	ClientInfo(POSITION p) : pos(p) {};
	
	void getPos(int& x, int& y, int& z) const { x = pos.x; y = pos.y; z = pos.z; }
	int getPosX() const { return pos.x; }
	int getPosY() const { return pos.y; }
	const SOCKET& getSocket() { return sock; }

	void setPos(int x, int y, int z) { pos.x = x; pos.y = y; pos.z = z; }
	void setPos(POSITION* p) { pos = *p; }
	void setSocket(SOCKET* s) { sock = *s; }
public:
	// ������԰� ������� ����
	ClientInfo(const ClientInfo&) = delete;
	ClientInfo& operator=(const ClientInfo&) = delete;
};

// ���� ���� ������ ���� ����ü�� ����
// recv�Ǵ� send�� �� ����� ���� �ӽð�ü
struct SockInfo
{
	WSAOVERLAPPED overlapped;
	WSABUF wsabuf;
};
struct CS_SOCK
{
	CS_SOCK() { cInfo = new ClientInfo; }
	~CS_SOCK() { delete cInfo; }
	
	u_short clientID;
	// �긦 �� �̷��� ����ϳ�?
	ClientInfo* cInfo;
	int key;
};

struct SC_SOCK
{
	// ���͸� ��°�� �ְ� �޾ƾ� �ϳ�?
	ClientInfo* cInfo;
};

SockInfo sInfo;
// accept()�� ���ϰ��� ������ ����. �� �����忡�� �����ϹǷ� ���� ������ ����.
SOCKET client_sock;
// client_sock ������ ��ȣ�ϱ� ���� �̺�Ʈ ��ü �ڵ�.
HANDLE hReadEvent, hWriteEvent;

std::vector<ClientInfo> vClient;
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
		CS_SOCK *ptr = new CS_SOCK;
		ptr->cInfo = new ClientInfo(POSITION(0, 0, 0));
		clientCount++;

		if (ptr == NULL)
		{
			printf("[����] �޸𸮰� �����մϴ�!\n");
			return 1;
		}
		ZeroMemory(&sInfo.overlapped, sizeof(sInfo.overlapped));
		ptr->cInfo->setSocket(&client_sock);
		// client_sock ���� ���� �о�� ��� hReadEent ��ȣ ���·� ��ȯ
		SetEvent(hReadEvent);
		// ������???? cInfo�� �����Ҵ��� ���������?
		// ���μ������� ������ �����ϰ� �ش� ��ü �����͸� �Ѱ��ٱ�?? 
		sInfo.wsabuf.buf = (char *)ptr->cInfo;
		sInfo.wsabuf.len = sizeof(ClientInfo);
		// ���� �޾ƿ����� client���� ������ ��� ���Ϳ� ����
		vClient.emplace_back(*ptr);

		// �񵿱� ����� ����
		DWORD recvbytes;
		DWORD flags = 0;
		retval = WSARecv(ptr->cInfo->getSocket(), &sInfo.wsabuf, 1, &recvbytes, &flags, &sInfo.overlapped, RecvCompletionRoutine);
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
	CS_SOCK *ptr = (CS_SOCK *)lpOverlapped;
	SOCKADDR_IN clientaddr;
	int addrlen = sizeof(clientaddr);

	{
		SOCKET tmpSock;
		getpeername(tmpSock, (SOCKADDR *)&clientaddr, &addrlen);
		ptr->cInfo->setSocket(&tmpSock);
	}

	// �񵿱� ����� ��� Ȯ��
	if (dwError != 0 || cbTransferred == 0)
	{
		if (dwError != 0)
			err_display(dwError);
		closesocket(ptr->cInfo->getSocket());
		printf("[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ� = %s, ��Ʈ ��ȣ = %d\n",
			inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
		delete ptr;
		return;
	}

	// recv ��ó�� �ڵ�
	// ���� key�� position�� ������ ����
	int key = 0;
	POSITION tmpPos;
	u_short cID;

	cID = ptr->clientID;
	key = ptr->key;
	ptr->cInfo->getPos(tmpPos.x, tmpPos.y, tmpPos.z);

	printf("ClientID : %d\nKey : %d\n", cID, key);

	switch (key)
	{
	case UP_KEY:
		printf("UP_KEY ����\n");
		tmpPos.y += 10;
		break;
	case DOWN_KEY:
		printf("DOWN_KEY ����\n");
		tmpPos.y -= 10;
		break;
	case RIGHT_KEY:
		printf("RIGHT_KEY ����\n");
		tmpPos.x += 10;
		break;
	case LEFT_KEY:
		printf("LEFT_KEY ����\n");
		tmpPos.x -= 10;
		break;
	}
	EnterCriticalSection(&cs);
	vClient[cID].setPos(&tmpPos);
	printf("ClientID : %d\nkey : %d\nx : %d, y : %d\n", cID, key, vClient[cID].getPosX(), vClient[cID].getPosY());
	LeaveCriticalSection(&cs);

	SC_SOCK *scSock = new SC_SOCK;
	ZeroMemory(&sInfo.overlapped, sizeof(sInfo.overlapped));
	// �̷��� �����ؾ��ϳ� scSock�� ������ �� �ֳ�
	sInfo.wsabuf.buf = (char *)scSock;
	sInfo.wsabuf.len = sizeof(SC_SOCK);
	
	// ����� ������ send
	DWORD sendbytes;
	for (int i = 0; i < vClient.size(); ++i)
	{
		retval = WSASend(vClient[i].getSocket(), &sInfo.wsabuf, 1, &sendbytes, 0, &sInfo.overlapped, SendCompletionRoutine);
		if (retval == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSA_IO_PENDING)
			{
				err_display("WSASend()");
				return;
			}
		}
	}
	// ������ �ޱ�
	// �̰� �� ���ָ� �̾ �ȵǴµ�. ����ؼ� recv�ޱ� ���ؼ��� �Ϸ��ƾ�� ��� ȣ���ؾ� �ϴ� �� ����.
	ZeroMemory(&sInfo.overlapped, sizeof(sInfo.overlapped));
	// �̷��� �����ؾ��ϳ� ptr ��ü�� ������ �� �ֳ�
	sInfo.wsabuf.buf = (char *)ptr;
	sInfo.wsabuf.len = sizeof(CS_SOCK);

	DWORD recvbytes;
	DWORD flags = 0;
	retval = WSARecv(ptr->cInfo->getSocket(), &sInfo.wsabuf, 1, &recvbytes, &flags, &sInfo.overlapped, RecvCompletionRoutine);
	if (retval == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			err_display("WSARecv()");
			return;
		}
	}
}

// SEND �񵿱� ����� ó�� �Լ�(����� �Ϸ� ��ƾ)
void CALLBACK SendCompletionRoutine(DWORD dwError, DWORD cbTransferred, LPWSAOVERLAPPED lpOverlapped, DWORD dwFlags)
{
	int retval;

	// Ŭ���̾�Ʈ ���� ���
	CS_SOCK *ptr = (CS_SOCK *)lpOverlapped;
	SOCKADDR_IN clientaddr;
	int addrlen = sizeof(clientaddr);
	getpeername(ptr->cInfo->getSocket(), (SOCKADDR *)&clientaddr, &addrlen);

	// �񵿱� ����� ��� Ȯ��
	if (dwError != 0 || cbTransferred == 0)
	{
		if (dwError != 0)
			err_display(dwError);
		closesocket(ptr->cInfo->getSocket());
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