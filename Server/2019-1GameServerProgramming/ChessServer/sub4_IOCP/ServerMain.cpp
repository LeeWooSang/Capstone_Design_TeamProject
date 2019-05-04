/*
## ���� ���� : 1 v n - overlapped callback
1. socket()            : ���ϻ���
2. bind()            : ���ϼ���
3. listen()            : ���Ŵ�⿭����
4. accept()            : ������
5. read()&write()
WIN recv()&send    : ������ �а���
6. close()
WIN closesocket    : ��������
*/

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <map>
#include <vector>
#include <set>
#include <thread>
#include <mutex>

#include <winsock2.h>
#include "protocol.h"
using namespace std;

#pragma comment(lib, "Ws2_32.lib")

#define MAX_BUFFER        1024

constexpr int SECTOR_WIDTH = 10;
constexpr int SECTOR_HEIGHT = 10;

constexpr int MAX_SECTOR_X = (WORLD_WIDTH + 0.5) / SECTOR_WIDTH;
constexpr int MAX_SECTOR_Y = (WORLD_HEIGHT + 0.5) / SECTOR_HEIGHT;

constexpr int PLAYER_INIT_X = 5;
constexpr int PLAYER_INIT_Y = 5;

constexpr int SCOPE_WIDTH = 7;
constexpr int SCOPE_HEIGHT = 7;

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
	char x, y;
	int nowSectorX;
	int nowSectorY;
	set<int> viewList;
public:
	SOCKETINFO() {
		in_use = false;
		over_ex.dataBuffer.len = MAX_BUFFER;
		over_ex.dataBuffer.buf = over_ex.messageBuffer;
		over_ex.is_recv = true;
	}
};


// <id, client>
// worker thread���� data race�߻�. ��ȣ���� �ʿ�
SOCKETINFO clients[MAX_USER];
set<int> sectors[MAX_SECTOR_X][MAX_SECTOR_Y];
HANDLE g_iocp;
mutex myLock;

void error_display(const char *msg, int err_no)
{
	WCHAR *lpMsgBuf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, err_no, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);
	cout << msg;
	wcout << L"���� [" << err_no << L"] " << lpMsgBuf << "\n";
	while (true);
	LocalFree(lpMsgBuf);
}

void do_recv(char id)
{
	DWORD flags = 0;

	if (WSARecv(clients[id].socket, &clients[id].over_ex.dataBuffer, 1, NULL, &flags, &(clients[id].over_ex.over), 0))
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			cout << "Error - IO pending Failure\n";
			while (true);
		}
	}
	else {
		// �񵿱������ ���ư��� �ʰ� ��������� ���ư��ٴ� ����.
		cout << "Non Overlapped Recv return.\n";
		while (true);
	}
}

void send_packet(char client, void *packet)
{
	char *p = reinterpret_cast<char *>(packet);
	OVER_EX *ov = new OVER_EX;
	ov->dataBuffer.len = p[0];
	ov->dataBuffer.buf = ov->messageBuffer;
	ov->is_recv = false;
	memcpy(ov->messageBuffer, p, p[0]);
	ZeroMemory(&ov->over, sizeof(ov->over));
	int error = WSASend(clients[client].socket, &ov->dataBuffer, 1, 0, 0, &ov->over, NULL);
	if (0 != error)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			cout << "Error - IO pending Failure\n";
			while (true);
		}
	}
	else {
		// �񵿱������ ���ư��� �ʰ� ��������� ���ư��ٴ� ����.
		//cout << "Non Overlapped Send return.\n";
		//while (true);
	}
}

void send_pos_packet(char client, char player)
{
	sc_packet_move_player packet;
	packet.id = player;
	packet.size = sizeof(packet);
	packet.type = SC_MOVE_PLAYER;
	packet.x = clients[player].x;
	packet.y = clients[player].y;

	send_packet(client, &packet);
}

void send_login_ok_packet(char new_id)
{
	sc_packet_login_ok packet;
	packet.id = new_id;
	packet.size = sizeof(packet);
	packet.type = SC_LOGIN_OK;

	send_packet(new_id, &packet);
}
void send_put_player_packet(char client, char new_id)
{
	sc_packet_put_player packet;
	packet.id = new_id;
	packet.size = sizeof(packet);
	packet.type = SC_PUT_PLAYER;
	packet.x = clients[new_id].x;
	packet.y = clients[new_id].y;

	send_packet(client, &packet);
}
void send_remove_player_packet(char client, char id)
{
	sc_packet_remove_player packet;
	packet.id = id;
	packet.size = sizeof(packet);
	packet.type = SC_REMOVE_PLAYER;

	send_packet(client, &packet);
}

void checkView(char client)
{
	set<int> nearList;
	set<int> removedID;

	// sector ��˻�
	int newSectorX = clients[client].x / 10;
	int newSectorY = clients[client].y / 10;
	if ((newSectorX != clients[client].nowSectorX) || (newSectorY != clients[client].nowSectorY))
	{
		sectors[clients[client].nowSectorY][clients[client].nowSectorX].erase(client);
		clients[client].nowSectorX = newSectorX;
		clients[client].nowSectorY = newSectorY;
		sectors[clients[client].nowSectorY][clients[client].nowSectorX].emplace(client);
	}
	printf("%d, %d\n", clients[client].nowSectorX, clients[client].nowSectorY);

	int startSectorX = clients[client].nowSectorX * 10;
	int startSectorY = clients[client].nowSectorY * 10;

	int startScopeX = clients[client].x - SCOPE_WIDTH / 2;
	int startScopeY = clients[client].y - SCOPE_HEIGHT / 2;
	
	int scopeLeft = startScopeX;
	int scopeRight = startScopeX + SCOPE_WIDTH;
	int scopeTop = startScopeY;
	int scopeBottom = startScopeY + SCOPE_HEIGHT;

	// �ڽ��� sector
	for (int d : sectors[clients[client].nowSectorY][clients[client].nowSectorX])
	{
		// �÷��̾ �þ� ���� �ȿ� ������
		if ((scopeRight > clients[d].x && scopeLeft <= clients[d].x)
			&& (scopeBottom > clients[d].y && scopeTop <= clients[d].y))
		{
			nearList.emplace(d);
		}
	}
	// Left
	if (scopeLeft < startSectorX && 0 < clients[client].nowSectorX)
	{
		for (int d : sectors[clients[client].nowSectorY][clients[client].nowSectorX - 1])
		{
			// �÷��̾ �þ� ���� �ȿ� ������
			if ((scopeRight > clients[d].x && scopeLeft <= clients[d].x)
				&& (scopeBottom > clients[d].y && scopeTop <= clients[d].y))
			{
				nearList.emplace(d);
			}
		}
	}
	// right
	if (scopeRight > startSectorX + SECTOR_WIDTH && MAX_SECTOR_X - 1 > clients[client].nowSectorX)
	{
		for (int d : sectors[clients[client].nowSectorY][clients[client].nowSectorX + 1])
		{
			// �÷��̾ �þ� ���� �ȿ� ������
			if ((scopeRight > clients[d].x && scopeLeft <= clients[d].x)
				&& (scopeBottom > clients[d].y && scopeTop <= clients[d].y))
			{
				nearList.emplace(d);
			}
		}
	}
	// top
	if (scopeTop < startSectorY && 0 < clients[client].nowSectorY)
	{
		for (int d : sectors[clients[client].nowSectorY - 1][clients[client].nowSectorX])
		{
			// �÷��̾ �þ� ���� �ȿ� ������
			if ((scopeRight > clients[d].x && scopeLeft <= clients[d].x)
				&& (scopeBottom > clients[d].y && scopeTop <= clients[d].y))
			{
				nearList.emplace(d);
			}
		}
	}
	// bottom
	if (scopeBottom > startSectorY + SECTOR_HEIGHT && MAX_SECTOR_Y - 1 > clients[client].nowSectorY)
	{
		for (int d : sectors[clients[client].nowSectorY + 1][clients[client].nowSectorX])
		{
			// �÷��̾ �þ� ���� �ȿ� ������
			if ((scopeRight > clients[d].x && scopeLeft <= clients[d].x)
				&& (scopeBottom > clients[d].y && scopeTop <= clients[d].y))
			{
				nearList.emplace(d);
			}
		}
	}


	// viewList�� nearList ��
	//printf("NearList: ");
	for (int i : nearList)
	{
		//printf("%d ", i);
		auto tmp = clients[client].viewList.find(i);
		// nearList�� �ְ� viewList�� ������ �߰�
		if (tmp == clients[client].viewList.cend())
		{
			clients[client].viewList.emplace(i);
			// ������ ��� �߰�
			send_put_player_packet(client, i);
		}

		// ��� viewList�� Ȯ��
		auto tmp2 = clients[i].viewList.find(client);
		if (tmp2 == clients[i].viewList.cend())
		{// ��� viewList�� ���� ������
			clients[i].viewList.emplace(client);
			// ��뿡�� �� �߰�
			send_put_player_packet(i, client);
		}
		else
		{// ��� viewList�� ���� ������
			// ��뿡�� ���� �̵����� �۽�
			send_pos_packet(i, client);
		}

	}
	//printf("\n");
	for (int i : clients[client].viewList)
	{
		auto tmp = nearList.find(i);
		// nearList�� ���� viewList�� ������ ����
		if (tmp == nearList.cend())
			removedID.emplace(i);
	}

	// viewList���� removedID�� �ִ� ��ü ����
	myLock.lock();
	for (int i : removedID)
	{
		clients[client].viewList.erase(i);
		send_remove_player_packet(client, i);

		auto tmp = clients[i].viewList.find(client);
		// ��� viewList�� ���� ������
		if (tmp != clients[i].viewList.cend())
		{
			// ��� viewList���� �� ����
			clients[i].viewList.erase(client);
			send_remove_player_packet(i, client);
		}
	}
	myLock.unlock();

}

void process_packet(char client, char* packet)
{
	cs_packet_up *p = reinterpret_cast<cs_packet_up *>(packet);
	int x = clients[client].x;
	int y = clients[client].y;

	// 0���� ������, 1���� ��ŶŸ��
	switch (p->type)
	{
	case CS_UP:
		if (y > 0)
			y--;
		break;
	case CS_DOWN:
		if (y < (WORLD_HEIGHT - 1))
			y++;
		break;
	case CS_LEFT:
		if (x > 0)
			x--;
		break;
	case CS_RIGHT:
		if (x < (WORLD_WIDTH - 1))
			x++;
		break;
	default:
		wcout << L"���ǵ��� ���� ��Ŷ ���� ����!!\n";
		while (true);
	}
	clients[client].x = x;
	clients[client].y = y;
	checkView(client);
	/*for (int i = 0; i < MAX_USER; ++i)
		if (true == clients[i].in_use)
			send_pos_packet(i, client);*/
}

void disconnect_client(char id)
{
	for (int i = 0; i < MAX_USER; ++i)
	{
		if (false == clients[i].in_use)
			continue;
		if (i == id)
			continue;
		send_remove_player_packet(i, id);
	}
	closesocket(clients[id].socket);
	clients[id].in_use = false;
}
void worker_thread()
{
	while (true)
	{
		DWORD io_byte;
		ULONGLONG l_key;
		// �������� �����͸� �Ѱ����
		OVER_EX *over_ex;

		int is_error = GetQueuedCompletionStatus(g_iocp, &io_byte, &l_key, reinterpret_cast<LPWSAOVERLAPPED *>(&over_ex), INFINITE);

		if (0 == is_error)
		{
			int err_no = WSAGetLastError();
			if (64 == err_no)
			{
				disconnect_client(l_key);
				continue;
			}
			else
				error_display("GQCS : ", err_no);
		}

		if (0 == io_byte)
		{
			disconnect_client(l_key);
			continue;
		}

		char key = static_cast<char>(l_key);
		if (true == over_ex->is_recv)
		{
			// RECV ó��
			wcout << "Packet from Client: " << key << "\n";
			// ��Ŷ����
			// ���� ũ��
			int rest = io_byte;
			// ���� ����
			char *ptr = over_ex->messageBuffer;
			char packet_size = 0;

			// ��Ŷ ������ �˾Ƴ��� (�߹ݺ��� ����)
			if (0 < clients[key].prev_size)
				packet_size = clients[key].packet_buffer[0];

			while (0 < rest) {
				if (0 == packet_size) packet_size = ptr[0];	// ptr[0]�� ���ݺ��� ó���� ��Ŷ
				// ��Ŷ ó���Ϸ��� �󸶳� �� �޾ƾ� �ϴ°�?
				// ������ ���� �������� ���� ��Ŷ�� ���� �� ������ prev_size ���ֱ�
				int required = packet_size - clients[key].prev_size;
				if (required <= rest) {
					// ��Ŷ ���� �� �ִ� ���

					// �տ� ���ִ� �����Ͱ� ����Ǿ����� �� ������ �� �ڿ� ����ǵ��� prev_size�� �����ش�.
					memcpy(clients[key].packet_buffer + clients[key].prev_size, ptr, required);
					process_packet(key, clients[key].packet_buffer);
					rest -= required;
					ptr += required;
					// �̹� ���Ʊ� ������ ���� ��Ŷ�� ó���� �� �ֵ��� 0
					packet_size = 0;
					clients[key].prev_size = 0;
				}
				else {
					// ��Ŷ ���� �� ���� ���
					memcpy(clients[key].packet_buffer + clients[key].prev_size, ptr, rest);
					rest = 0;
					clients[key].prev_size += rest;
				}
			}

			do_recv(key);
		}
		else
		{
			// SEND ó��
			delete over_ex;
		}
	}
}

int do_accept()
{
	// Winsock Start - windock.dll �ε�
	WSADATA WSAData;
	if (WSAStartup(MAKEWORD(2, 2), &WSAData) != 0)
	{
		cout << "Error - Can not load 'winsock.dll' file\n";
		return 1;
	}

	// 1. ���ϻ���											Overlapped�ҰŸ� ������ WSA_FLAG_OVERLAPPED
	SOCKET listenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (listenSocket == INVALID_SOCKET)
	{
		cout << "Error - Invalid socket\n";
		return 1;
	}

	// �������� ��ü����
	SOCKADDR_IN serverAddr;
	memset(&serverAddr, 0, sizeof(SOCKADDR_IN));
	serverAddr.sin_family = PF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	// 2. ���ϼ���
	// std�� bind�� ȣ��ǹǷ� ������ bind�� �ҷ��ֱ� ���� �տ� ::����
	if (::bind(listenSocket, (struct sockaddr*)&serverAddr, sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
	{
		cout << "Error - Fail bind\n";
		// 6. ��������
		closesocket(listenSocket);
		// Winsock End
		WSACleanup();
		return 1;
	}

	// 3. ���Ŵ�⿭����
	if (listen(listenSocket, 5) == SOCKET_ERROR)
	{
		cout << "Error - Fail listen\n";
		// 6. ��������
		closesocket(listenSocket);
		// Winsock End
		WSACleanup();
		return 1;
	}

	SOCKADDR_IN clientAddr;
	int addrLen = sizeof(SOCKADDR_IN);
	memset(&clientAddr, 0, addrLen);
	SOCKET clientSocket;
	DWORD flags;

	// accept() ���ʹ� ���� ���� ��ƾ
	while (1)
	{
		// clientSocket�� �񵿱������ ����� ���ؼ��� listenSocket�� �񵿱���̾�� �Ѵ�.
		clientSocket = accept(listenSocket, (struct sockaddr *)&clientAddr, &addrLen);
		if (clientSocket == INVALID_SOCKET)
		{
			cout << "Error - Accept Failure\n";
			return 1;
		}

		// id�� 0~9���� ����� ����
		int new_id = -1;
		for (int i = 0; i < MAX_USER; ++i)
		{
			// i�� �ε����� �ϴ� ���� �� ���� �ִ°�?
			if (false == clients[i].in_use)
			{
				new_id = i;
				break;
			}
		}
		if (-1 == new_id)
		{
			cout << "MAX USER overflow\n";
			continue;
		}
		// �ӽð�ü�� ��������ϹǷ� ���ư�����
		//clients[new_id] = SOCKETINFO{clientSocket};
		// Solution 1. emplace�� ���� : �� ���� �ְ� �� �� ���� �ִ�. (������ ���� �� �� ����)
		// Solution 2. over_ex�� new�� ���� : �ڵ尡 ��������.
		// Solution 3. clients�� map�� �ƴ� �迭�� ���� : null �����ڸ� �߰��ؾ� �Ѵ�.

		// ������ ��� ���
		// ������ �����ϰ� �����ְ� ���� ���̰� ���������� �ȵǱ� ������ �ʱ�ȭ
		clients[new_id].socket = clientSocket;
		clients[new_id].prev_size = 0;
		clients[new_id].x = PLAYER_INIT_X;
		clients[new_id].y = PLAYER_INIT_Y;
		clients[new_id].nowSectorX = PLAYER_INIT_X / 10;
		clients[new_id].nowSectorY = PLAYER_INIT_Y / 10;
		ZeroMemory(&clients[new_id].over_ex.over, sizeof(clients[new_id].over_ex.over));

		flags = 0;

		CreateIoCompletionPort(reinterpret_cast<HANDLE>(clientSocket), g_iocp, new_id, 0);

		// �길 �ʰ��ϴ� ������ true���ִ� ���� send�� �� ���ư��µ�
		// IOCP���� �����̹Ƿ� IOCP �ݹ��� ���� �� �� �޸� ������ �Ͼ�� ����
		// ���� IOCP�� ���� ����� �� �ֵ��� CICP�Լ� ���Ŀ� true�� ����
		// CICP������ ���ָ� ������ recv�� ����
		clients[new_id].in_use = true;

		send_login_ok_packet(new_id);
		sectors[clients[new_id].nowSectorX][clients[new_id].nowSectorY].emplace(new_id);
		checkView(new_id);

		do_recv(new_id);
	}

	// 6-2. ���� ��������
	closesocket(listenSocket);

	// Winsock End
	WSACleanup();

	return 0;
}

int main()
{
	vector <thread> worker_threads;

	// error_display�� ����
	wcout.imbue(locale("korean"));
	// iocp ����
	g_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
	// worker_Thread����
	for (int i = 0; i < 4; ++i)
		worker_threads.emplace_back(thread{ worker_thread });
	thread accept_thread{ do_accept };
	accept_thread.join();
	for (auto &th : worker_threads) th.join();
}