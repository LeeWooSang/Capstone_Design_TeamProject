#include "Network.h"
#include "../GameFramework/GameFramework.h"

#pragma warning(disable : 4996)

#ifdef _WITH_SERVER_
Network::Network()
{
	sock = NULL;
	myId = -1;
	in_packet_size = 0;
	saved_packet_size = 0;
}

Network::~Network()
{
	m_pGameClient = nullptr;
}

bool Network::connectToServer(HWND hWnd)
{
	// ���� IP�ּ� �Է¹ޱ�
	std::string s;
	printf("����IP�Է� : ");
	std::cin >> s;
	const char *serverIp = s.c_str();

	// ���� �ʱ�ȭ
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		PostQuitMessage(0);

	// socket()
	sock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);
	if (sock == INVALID_SOCKET)
	{
		err_quit("socket()");
		return false;
	}

	// connect()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	//serveraddr.sin_addr.s_addr = inet_addr(SERVER_IP);
	serveraddr.sin_addr.s_addr = inet_addr(serverIp);
	serveraddr.sin_port = htons(SERVER_PORT);
	int retval = WSAConnect(sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr), NULL, NULL, NULL, NULL);
	if (retval == SOCKET_ERROR)
	{
		if (GetLastError() != WSAEWOULDBLOCK)
			err_quit("connect()");
		return false;
	}

	WSAAsyncSelect(sock, hWnd, WM_SOCKET, FD_CLOSE | FD_READ);

	send_wsabuf.buf = send_buffer;
	send_wsabuf.len = BUF_SIZE;
	recv_wsabuf.buf = recv_buffer;
	recv_wsabuf.len = BUF_SIZE;

	return true;
}

SOCKET Network::getSock()
{
	return sock;
}

void Network::ReadPacket()
{
	DWORD iobyte, ioflag = 0;

	int retval = WSARecv(sock, &recv_wsabuf, 1, &iobyte, &ioflag, NULL, NULL);
	if (retval) {
		err_display("WSARecv()");
	}

	BYTE *ptr = reinterpret_cast<BYTE *>(recv_buffer);

	while (0 != iobyte)
	{
		if (0 == in_packet_size)
			in_packet_size = ptr[0];

		int required = in_packet_size - saved_packet_size;

		if (iobyte + saved_packet_size >= in_packet_size)
		{// �ϼ��� �� ���� ��
			memcpy(packet_buffer + saved_packet_size, ptr, required);
			
			if (m_pGameClient)
			{
				m_pGameClient->ProcessPacket(packet_buffer);
			}
			// �� Scene�� ProcessPacket���� ó���� �ѱ�
			//ProcessPacket(packet_buffer);
			ptr += required;
			iobyte -= required;
			in_packet_size = 0;
			saved_packet_size = 0;
		}
		else
		{// �ϼ� �� �� ��
			memcpy(packet_buffer + saved_packet_size, ptr, iobyte);
			saved_packet_size += iobyte;
			iobyte = 0;
		}
	}
}



void Network::SendPacket()
{
	DWORD iobyte = 0;
	int retval = WSASend(sock, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);
	if (retval)
	{
		if (GetLastError() != WSAEWOULDBLOCK)
		{
			err_display("WSASend()");
		}
	}
}

void Network::SendUpKey()
{
	pUp = reinterpret_cast<CS_PACKET_UP_KEY *>(send_buffer);
	pUp->size = sizeof(pUp);
	send_wsabuf.len = sizeof(pUp);
	pUp->type = CS_UP_KEY;

	SendPacket();
}
void Network::SendDownKey()
{
	pDown = reinterpret_cast<CS_PACKET_DOWN_KEY *>(send_buffer);
	pDown->size = sizeof(pDown);
	send_wsabuf.len = sizeof(pDown);
	pDown->type = CS_DOWN_KEY;

	SendPacket();
}
void Network::SendRightKey()
{
	pRight = reinterpret_cast<CS_PACKET_RIGHT_KEY *>(send_buffer);
	pRight->size = sizeof(pRight);
	send_wsabuf.len = sizeof(pRight);
	pRight->type = CS_RIGHT_KEY;

	SendPacket();
}
void Network::SendLeftKey()
{
	pLeft = reinterpret_cast<CS_PACKET_LEFT_KEY *>(send_buffer);
	pLeft->size = sizeof(pLeft);
	send_wsabuf.len = sizeof(pLeft);
	pLeft->type = CS_LEFT_KEY;

	SendPacket();
}
void Network::SendReady(int matID)
{
	pReady = reinterpret_cast<CS_PACKET_READY *>(send_buffer);
	pReady->matID = matID;
	pReady->size = sizeof(pReady);
	send_wsabuf.len = sizeof(pReady);
	pReady->type = CS_READY;
	printf("Send matID : %d\n", pReady->matID);
	SendPacket();
}
void Network::SendReqStart()
{
	pRequestStart = reinterpret_cast<CS_PACKET_REQUEST_START *>(send_buffer);
	pRequestStart->size = sizeof(pRequestStart);
	send_wsabuf.len = sizeof(pRequestStart);
	pRequestStart->type = CS_REQUEST_START;

	SendPacket();
}

void Network::SendReleaseKey()
{
	pReleaseKey = reinterpret_cast<CS_PACKET_RELEASE_KEY *>(send_buffer);
	pReleaseKey->size = sizeof(pReleaseKey);
	send_wsabuf.len = sizeof(pReleaseKey);
	pReleaseKey->type = CS_RELEASE_KEY;

	SendPacket();
}

void Network::SetGameFrameworkPtr(HWND hWnd,CGameFramework* client)
{
	if (client) 
	{
		m_pGameClient = client;
	}
}
// ���� �Լ� ���� ��� �� ����
void Network::err_quit(const char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, (LPCWSTR)msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);
}

// ���� �Լ� ���� ���
void Network::err_display(const char *msg)
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

#endif