#pragma comment(lib, "ws2_32")
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <iostream>
#include <conio.h>

#define SERVERIP "127.0.0.1"
#define SERVERPORT 9000
#define BUFSIZE    512

const static int UP_KEY = 72;
const static int DOWN_KEY = 80;
const static int RIGHT_KEY = 77;
const static int LEFT_KEY = 75;

using namespace std;

struct POSITION
{
	POSITION() :x(0), y(0), z(0) {};
	POSITION(int x1, int y1, int z1) :x(x1), y(y1), z(z1) {};
	int x, y, z;
};

struct CS_SOCK
{
	u_short clientID;
	int key;
};

struct SC_SOCK
{
	// ���͸� ��°�� �ְ� �޾ƾ� �ϳ�?
	POSITION pos;
};

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

// ����� ���� ������ ���� �Լ�
int recvn(SOCKET s, char *buf, int len, int flags)
{
	int received;
	char *ptr = buf;
	int left = len;

	while (left > 0)
	{
		received = recv(s, ptr, left, flags);
		if (received == SOCKET_ERROR)
			return SOCKET_ERROR;
		else if (received == 0)
			break;
		left -= received;
		ptr += received;
		cout << left << endl;
	}
	return (len - left);
}

int main(int argc, char *argv[])
{
	int retval;
	// ���� �ʱ�ȭ
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// socket()
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) err_quit("socket()");

	// connect()
	// ������ ���� ��û�ϰ� �ϳ��� ��ǻ�� �󿡼� ������ Ŭ���̾�Ʈ�� �����Ѵٴ� �����Ͽ�
	// IP�ּҸ� Roll back�ϴ� 127.0.0.1�� ����ϰ�, ��Ʈ��ȣ�� 9000������ ����
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr(SERVERIP);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = connect(sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR)
		err_quit("connect()");

	// ������ ��ſ� ����� ����
	//char buf[BUFSIZE + 1];
	int len;
	POSITION myPos(0, 0, 0);
	CS_SOCK csSock;

	// ������ ������ ���
	while (1)
	{
		// �������� send���ִ� �ڵ� �ۼ� �ʿ�
		retval = recv(sock, (char *)&csSock.clientID, sizeof(csSock.clientID), 0);
		if (retval == SOCKET_ERROR)
		{
			err_display("recv()");
			break;
		}
		// getch()�� Ű�Է� ������ ���Ѵ�� ����. but kbhit()�� ������ ����ħ -> kbhit�� �����ֱ�
		if (_kbhit())
		{
			int key; 
			key = _getch();
			if (key == 224 || key == 0)
			{
				key = _getch();
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
				csSock.key = key;

				printf("ClientID : %d\nkey : %d\n", csSock.clientID, csSock.key);
				retval = send(sock, (char *)&csSock.key, sizeof(csSock.key), 0);
				if (retval == SOCKET_ERROR)
				{
					err_display("send()");
					break;
				}

				// �������� send���ִ� ���� ��� recv�ؼ� �����ϴ� �ڵ� �ʿ�
			}

		}
		//// ������ �Է�
		//cout << endl;
		//cout << "[���� ������] ";
		//if (fgets(buf, BUFSIZE + 1, stdin) == NULL)
		//	break;

		//// '\n' ���� ���� �� NULL�� ä��
		//len = strlen(buf);
		////if (buf[len - 1] == '\n')
		////	buf[len - 1] = '\0';
		//if (strlen(buf) == 0)
		//	break;

		//// ������ ������
		//retval = send(sock, buf, strlen(buf), 0);
		//if (retval == SOCKET_ERROR)
		//{
		//	err_display("send()");
		//	break;
		//}
		//cout << "[TCP Ŭ���̾�Ʈ] " << retval << "����Ʈ�� ���½��ϴ�." << endl;

		//// ������ �ޱ�
		//retval = recvn(sock, buf, retval, 0);
		//if (retval == SOCKET_ERROR)
		//{
		//	err_display("recvn()");
		//	break;
		//}
		//else if (retval == 0)
		//	break;

		//// ���� ������ ���
		//buf[retval] = '\0';
		//cout << "[TCP Ŭ���̾�Ʈ] " << retval << "����Ʈ�� �޾ҽ��ϴ�." << endl;
		//cout << "[���� ������] " << buf << endl;
	}

	// closesocket()
	closesocket(sock);

	// ���� ����
	WSACleanup();
	return 0;
}
