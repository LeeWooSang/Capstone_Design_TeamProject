#include "Network.h"
#include "../GameFramework/GameFramework.h"

#pragma warning(disable : 4996)

#ifdef _WITH_SERVER_

//volatile bool g_LoginFinished;
//const char* g_serverIP = nullptr;

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


		// ���� �ʱ�ȭ
	bool ret = false;
	if (m_connect)
	{
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
		serveraddr.sin_addr.s_addr = inet_addr(m_ServerIP);
		serveraddr.sin_port = htons(SERVER_PORT);
		int retval = WSAConnect(sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr), NULL, NULL, NULL, NULL);
		if (retval == SOCKET_ERROR)
		{
			if (GetLastError() != WSAEWOULDBLOCK)
			{
				//g_LoginFinished = true;
				err_quit("connect()");
			}
			return false;
		}

		WSAAsyncSelect(sock, hWnd, WM_SOCKET, FD_CLOSE | FD_READ);

		send_wsabuf.buf = send_buffer;
		send_wsabuf.len = BUF_SIZE;
		recv_wsabuf.buf = recv_buffer;
		recv_wsabuf.len = BUF_SIZE;
		ret = true;
		//g_LoginFinished = true;
	}
	
	return ret;
	
	
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

//8����Ʈ �̻��϶��� �� SendPacket�� ����Ͽ����Ѵ�.
void Network::SendPacket(DWORD dataBytes)
{
	DWORD iobyte = 0;
	
	send_wsabuf.len = dataBytes;
	int retval = WSASend(sock, &send_wsabuf, 1, &dataBytes, 0, NULL, NULL);
	if (retval)
	{
		if (GetLastError() != WSAEWOULDBLOCK)
		{
			err_display("WSASend()");
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

void Network::SendUpRightKey()
{
	pUp = reinterpret_cast<CS_PACKET_UP_KEY *>(send_buffer);
	pUp->size = sizeof(pUp);
	send_wsabuf.len = sizeof(pUp);
	pUp->type = CS_UPRIGHT_KEY;

	SendPacket();
}
void Network::SendUpLeftKey()
{
	pUp = reinterpret_cast<CS_PACKET_UP_KEY *>(send_buffer);
	pUp->size = sizeof(pUp);
	send_wsabuf.len = sizeof(pUp);
	pUp->type = CS_UPLEFT_KEY;

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
void Network::SendDownRightKey()
{
	pDown = reinterpret_cast<CS_PACKET_DOWN_KEY *>(send_buffer);
	pDown->size = sizeof(pDown);
	send_wsabuf.len = sizeof(pDown);
	pDown->type = CS_DOWNRIGHT_KEY;

	SendPacket();
}
void Network::SendDownLeftKey()
{
	pDown = reinterpret_cast<CS_PACKET_DOWN_KEY *>(send_buffer);
	pDown->size = sizeof(pDown);
	send_wsabuf.len = sizeof(pDown);
	pDown->type = CS_DOWNLEFT_KEY;

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

void Network::SendNotReady()
{
	pUnReady = reinterpret_cast<CS_PACKET_UNREADY*>(send_buffer);
	
	pUnReady->size = sizeof(pUnReady);
	pUnReady->type = CS_UNREADY;
	send_wsabuf.len = sizeof(pUnReady);

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
void Network::SendAnimationState(char animNum)
{
	pAnimation = reinterpret_cast<CS_PACKET_ANIMATION*>(send_buffer);
	pAnimation->size = sizeof(pAnimation);
	send_wsabuf.len = sizeof(pAnimation);
	pAnimation->type = CS_ANIMATION_INFO;
	pAnimation->animation = animNum;

	SendPacket();
}

void Network::SendChattingText(char id,const _TCHAR *text)
{
	pText = reinterpret_cast<CS_PACKET_CHATTING*>(send_buffer);
	pText->size = sizeof(CS_PACKET_CHATTING);
	pText->type = CS_CHATTING;
	pText->id = id;
	pText->padding = 0;
	
	int nLen = WideCharToMultiByte(CP_ACP, 0, text, -1, NULL, 0, NULL, NULL);

	WideCharToMultiByte(CP_ACP, 0, text, -1, pText->chatting, nLen, NULL, NULL);

	SendPacket(pText->size);
}
void Network::SendNickName(char id,_TCHAR* name)
{
	pNickName = reinterpret_cast<CS_PACKET_NICKNAME*>(send_buffer);
	pNickName->size = sizeof(CS_PACKET_NICKNAME);
	pNickName->type = CS_NICKNAME_INFO;
	pNickName->id = id;
	pNickName->padding = 0;

	int nLen = WideCharToMultiByte(CP_ACP, 0, name, -1, NULL, 0,NULL,NULL);

	WideCharToMultiByte( CP_ACP, 0, name, -1, pNickName->name, nLen, NULL, NULL );


	SendPacket(pNickName->size);
}
void Network::SetGameFrameworkPtr(HWND hWnd,CGameFramework* client)
{
	if (client) 
	{
		m_pGameClient = client;
	}
}
void Network::SendSurroundingCollision(USHORT objID)
{
	pCollide = reinterpret_cast<CS_PACKET_OBJECT_COLLISION*>(send_buffer);
	pCollide->objId = objID;
	pCollide->size = sizeof(pCollide);
	send_wsabuf.len = sizeof(pCollide);
	pCollide->type = CS_OBJECT_COLLISION;

	SendPacket();
}

void Network::SendNotSurroundingCollision()
{
	pNotCollide = reinterpret_cast<CS_PACKET_NOT_OBJECT_COLLISION*>(send_buffer);
	pNotCollide->size = sizeof(pNotCollide);
	pNotCollide->type = CS_NOT_OBJECT_COLLISION;
	send_wsabuf.len = sizeof(pNotCollide);

	SendPacket();
}

void Network::SendPlayerCollision(unsigned char playerID)
{
	pPlayerCollision = reinterpret_cast<CS_PACKET_PLAYER_COLLISION*>(send_buffer);
	pPlayerCollision->size = sizeof(pPlayerCollision);
	pPlayerCollision->type = CS_PLAYER_COLLISION;
	pPlayerCollision->playerID = playerID;
	send_wsabuf.len = sizeof(pPlayerCollision);

	SendPacket();
}

void Network::SendNotPlayerCollision(unsigned char playerID)
{
	pPlayerNotCollision = reinterpret_cast<CS_PACKET_NOT_PLAYER_COLLISION*>(send_buffer);
	pPlayerNotCollision->size = sizeof(pPlayerCollision);
	pPlayerNotCollision->type = CS_NOT_PLAYER_COLLISION;
	pPlayerNotCollision->playerID = playerID;
	send_wsabuf.len - sizeof(pPlayerNotCollision);

	SendPacket();
}

void Network::SendUseItem(int useItem, int targetID)
{
	pItem = reinterpret_cast<CS_PACKET_USE_ITEM*>(send_buffer);
	pItem->usedItem = useItem;
	pItem->target = targetID;
	pItem->size = sizeof(pItem);
	send_wsabuf.len = sizeof(pItem);
	pItem->type = CS_USEITEM;

	SendPacket();
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
	//exit(1);
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