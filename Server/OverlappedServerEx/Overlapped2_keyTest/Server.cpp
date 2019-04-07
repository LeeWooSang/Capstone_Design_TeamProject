#pragma comment(lib, "ws2_32")
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#include <stdlib.h>
#include <iostream>
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

// ���� ���� ������ ���� ����ü�� ����
// recv�Ǵ� send�� �� ����� ���� �ӽð�ü
struct SC_INIT_SOCK
{
	int clientID;
	POSITION pos;
};

struct CS_SOCK
{
	int clientID;
	int key;
};

struct SC_SOCK
{
	// ���͸� ��°�� �ְ� �޾ƾ� �ϳ�?
	POSITION pos;
};

// Ŭ���̾�Ʈ���� ������ ��Ƴ��� ������ ��ü
// C++�� class�� �ϴ� ���� �� �� ���۵� �� ����
// WSAOVERLAPPED�� �ε����� ���� map���� ����ų�, struct�� �ۼ�
struct ClientInfo
{
public:
	WSAOVERLAPPED overlapped;
	WSABUF wsabuf;
	CS_SOCK csSock;
	POSITION pos;
	SOCKET sock;

public:
	ClientInfo() :pos(0, 0, 0){};
	ClientInfo(POSITION p) : pos(p) {};
	
public:
	// ������԰� ������� ����
	//ClientInfo(const ClientInfo&) = delete;
	ClientInfo& operator=(const ClientInfo&) = delete;
};



// accept()�� ���ϰ��� ������ ����. �� �����忡�� �����ϹǷ� ���� ������ ����.
SOCKET client_sock;
// client_sock ������ ��ȣ�ϱ� ���� �̺�Ʈ ��ü �ڵ�.
//HANDLE hReadEvent, hWriteEvent;

SC_SOCK scSock;
SC_INIT_SOCK scInitSock;

std::vector<ClientInfo> vClient;
u_short clientCount;
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

	//// �̺�Ʈ ��ü ����
	//// CreateEvent(����, true->�������� / false->�ڵ�����, true->��ȣ�ν��� / false->���ȣ�ν���, �̺�Ʈ�̸�)
	//// WorkerThread�� client_sock�� ���� �о����� main �����忡 �˸� (�б� �˸�)
	//hReadEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
	//if (hReadEvent == NULL)
	//	return 1;
	//// main �����尡 client_sock�� ���� ���������� alertable wait ������ WorkerThread�� �˸� (���� �˸�)
	//hWriteEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	//if (hWriteEvent == NULL)
	//	return 1;

	// ������ ����
	// C++11 ǥ�� ���̺귯�� ������ #include <thread> �� ���� ���
	/*HANDLE hThread = CreateThread(NULL, 0, WorkerThread, NULL, 0, NULL);
	if (hThread == NULL)
		return 1;
	CloseHandle(hThread);*/

	while (1)
	{
		// accept()
		client_sock = accept(listen_sock, NULL, NULL);
		if (client_sock == INVALID_SOCKET)
		{
			err_display("accept()");
			break;
		}
		// ������ Ŭ���̾�Ʈ ���� ���
		SOCKADDR_IN clientaddr;
		int addrlen = sizeof(clientaddr);
		getpeername(client_sock, (SOCKADDR *)&clientaddr, &addrlen);
		printf("\n[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ� = %s, ��Ʈ ��ȣ = %d\n",
			inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

		// ���� ���� ����ü �Ҵ�� �ʱ�ȭ
		ClientInfo *ptr = new ClientInfo(POSITION(0, 0, 0));
		if (ptr == NULL)
		{
			printf("[����] �޸𸮰� �����մϴ�!\n");
			return 1;
		}

		ZeroMemory(&ptr->overlapped, sizeof(ptr->overlapped));
		ptr->sock = client_sock;
		vClient.emplace_back(*ptr);

		scInitSock.clientID = clientCount++;
		scInitSock.pos = ptr->pos;
		ptr->wsabuf.buf = (char *)&scInitSock;
		ptr->wsabuf.len = sizeof(SC_INIT_SOCK);

		// ClientID send
		retval = WSASend(ptr->sock, &ptr->wsabuf, 1, NULL, 0, &ptr->overlapped, SendCompletionRoutine);
		if (retval == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSA_IO_PENDING)
			{
				err_display("WSASend()");
				return 1;
			}
		}

		ZeroMemory(&ptr->overlapped, sizeof(ptr->overlapped));
		// client_sock ���� ���� �о�� ��� hReadEent ��ȣ ���·� ��ȯ
		// ���� accept()�� �ֿ� �� accept()�ؼ� �����ߴ� ��
		// ù send�� setEvent ���� ���� �����ؾ� �� �� ����.

		ptr->wsabuf.buf = (char *)&ptr->csSock;
		ptr->wsabuf.len = sizeof(CS_SOCK);
		// ���� �޾ƿ����� client���� ������ ��� ���Ϳ� ����
		std::cout << client_sock << "\n";
		// �񵿱� ����� ����
		DWORD flags = 0;
		retval = WSARecv(ptr->sock, &ptr->wsabuf, 1, NULL, &flags, &ptr->overlapped, RecvCompletionRoutine);
		if (retval == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSA_IO_PENDING)
			{
				err_display("WSARecv()");
				return 1;
			}
		}
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
		//while (1)
		//{
		//	// alertable wait
		//	DWORD result = WaitForSingleObjectEx(hWriteEvent, INFINITE, TRUE);
		//	// ���ο� Ŭ���̾�Ʈ�� ������ ��� -> ������ �����.
		//	if (result == WAIT_OBJECT_0)
		//		break;
		//	// WAIT_IO_COMPLETION: (�񵿱� ����� �۾� -> )�Ϸ� ��ƾ ȣ�� �Ϸ� -> �ٽ� alertable wait ���¿� ����.
		//	if (result != WAIT_IO_COMPLETION)
		//		return 1;
		//}

		//// ������ Ŭ���̾�Ʈ ���� ���
		//SOCKADDR_IN clientaddr;
		//int addrlen = sizeof(clientaddr);
		//getpeername(client_sock, (SOCKADDR *)&clientaddr, &addrlen);
		//printf("\n[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ� = %s, ��Ʈ ��ȣ = %d\n",
		//	inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

		//// ���� ���� ����ü �Ҵ�� �ʱ�ȭ
		//ClientInfo *ptr = new ClientInfo(POSITION(0, 0, 0));
		//if (ptr == NULL)
		//{
		//	printf("[����] �޸𸮰� �����մϴ�!\n");
		//	return 1;
		//}

		////ZeroMemory(&ptr->overlapped, sizeof(ptr->overlapped));
		////

		////scInitSock.clientID = clientCount++;
		////ptr->getPos(scInitSock.pos.x, scInitSock.pos.y, scInitSock.pos.z);
		////ptr->wsabuf.buf = (char *)&scInitSock;
		////ptr->wsabuf.len = sizeof(SC_INIT_SOCK);

		////// ClientID send
		////retval = WSASend(ptr->getSocket(), &ptr->wsabuf, 1, NULL, 0, &ptr->overlapped, SendCompletionRoutine);
		////if (retval == SOCKET_ERROR)
		////{
		////	if (WSAGetLastError() != WSA_IO_PENDING)
		////	{
		////		err_display("WSASend()");
		////		return 1;
		////	}
		////}

		//ZeroMemory(&ptr->overlapped, sizeof(ptr->overlapped));
		//ptr->sock = client_sock;
		//// client_sock ���� ���� �о�� ��� hReadEent ��ȣ ���·� ��ȯ
		//// ���� accept()�� �ֿ� �� accept()�ؼ� �����ߴ� ��
		//// ù send�� setEvent ���� ���� �����ؾ� �� �� ����.
		//SetEvent(hReadEvent);
		//vClient.emplace_back(*ptr);

		//ptr->wsabuf.buf = (char *)&ptr->csSock;
		//ptr->wsabuf.len = sizeof(CS_SOCK);
		//// ���� �޾ƿ����� client���� ������ ��� ���Ϳ� ����

		//// �񵿱� ����� ����
		//DWORD flags = 0;
		//retval = WSARecv(ptr->sock , &ptr->wsabuf, 1, NULL, &flags, &ptr->overlapped, RecvCompletionRoutine);
		//if (retval == SOCKET_ERROR)
		//{
		//	if (WSAGetLastError() != WSA_IO_PENDING)
		//	{
		//		err_display("WSARecv()");
		//		return 1;
		//	}
		//}
	}
	return 0;
}

// RECV �񵿱� ����� ó�� �Լ�(����� �Ϸ� ��ƾ)
void CALLBACK RecvCompletionRoutine(DWORD dwError, DWORD cbTransferred, LPWSAOVERLAPPED lpOverlapped, DWORD dwFlags)
{
	int retval;

	// Ŭ���̾�Ʈ ���� ���
	// �� �κ��� ��� �̾������� �𸣰���...
	ClientInfo *ptr = (ClientInfo *)lpOverlapped;
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
		return;
	}

	// recv ��ó�� �ڵ�
	// ���� key�� position�� ������ ����
	int key = 0;
	POSITION tmpPos;
	u_short cID;

	cID = ptr->csSock.clientID;
	key = ptr->csSock.key;
	tmpPos = ptr->pos;
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
	ptr->pos = tmpPos;
	vClient[cID].pos = ptr->pos;
	printf("ClientID : %d, key : %d\nx : %d, y : %d\n", cID, key, vClient[cID].pos.x, vClient[cID].pos.y);
	LeaveCriticalSection(&cs);

	// �ϴ� �غ��� �̻��ϸ� ClientInfo�� SC_SOCK��ü ����� �� ��.
	scSock.pos = tmpPos;
	
	ZeroMemory(&ptr->overlapped, sizeof(ptr->overlapped));
	ptr->wsabuf.buf = (char *)&scSock;
	ptr->wsabuf.len = sizeof(SC_SOCK);
	
	// ����� ������ send
	for (int i = 0; i < vClient.size(); ++i)
	{
		retval = WSASend(vClient[i].sock, &ptr->wsabuf, 1, NULL, 0, &ptr->overlapped, SendCompletionRoutine);
		if (retval == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSA_IO_PENDING)
			{
				err_display("WSASend()inCR");
				return;
			}
		}
	}
	// ������ �ޱ�
	// �̰� �� ���ָ� �̾ �ȵǴµ�. ����ؼ� recv�ޱ� ���ؼ��� �Ϸ��ƾ�� ��� ȣ���ؾ� �ϴ� �� ����.
	ZeroMemory(&ptr->overlapped, sizeof(ptr->overlapped));
	// �̷��� �����ؾ��ϳ� ptr ��ü�� ������ �� �ֳ�
	ptr->wsabuf.buf = (char *)&ptr->csSock;
	ptr->wsabuf.len = sizeof(CS_SOCK);

	DWORD flags = 0;
	retval = WSARecv(ptr->sock, &ptr->wsabuf, 1, NULL, &flags, &ptr->overlapped, RecvCompletionRoutine);
	if (retval == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			err_display("WSARecv()inCR");
			return;
		}
	}
}

// SEND �񵿱� ����� ó�� �Լ�(����� �Ϸ� ��ƾ)
void CALLBACK SendCompletionRoutine(DWORD dwError, DWORD cbTransferred, LPWSAOVERLAPPED lpOverlapped, DWORD dwFlags)
{
	int retval;

	// Ŭ���̾�Ʈ ���� ���
	ClientInfo *ptr = (ClientInfo *)lpOverlapped;
	SOCKADDR_IN clientaddr;
	int addrlen = sizeof(clientaddr);
	getpeername(ptr->sock, (SOCKADDR *)&clientaddr, &addrlen);

	// �񵿱� ����� ��� Ȯ��
	// ���⼭ ������ �� ���ε� ����. �̹� bind�� ���Ͽ� ���ε��ϰų� �ּ�ü�谡 �ϰ������� ���� ��
	// �ּ� ü�谡 �ϰ������� ������ �ƴұ�?
	if (dwError != 0)
	{
		err_display(dwError);
		closesocket(ptr->sock);
		printf("[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ� = %s, ��Ʈ ��ȣ = %d\n",
			inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
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