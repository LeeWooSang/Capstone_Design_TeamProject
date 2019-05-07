#include "Server.h"


Server::Server()
{
	//clients.reserve(MAX_USER);
	workerThreads.reserve(MAX_WORKER_THREAD);
}


Server::~Server()
{
	//clients.clear();
}

bool Server::InitServer()
{
	// Winsock Start - windock.dll �ε�
	WSADATA WSAData;
	if (WSAStartup(MAKEWORD(2, 2), &WSAData) != 0)
	{
		cout << "Error - Can not load 'winsock.dll' file\n";
		return false;
	}

	// 1. ���ϻ���											Overlapped�ҰŸ� ������ WSA_FLAG_OVERLAPPED
	listenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (listenSocket == INVALID_SOCKET)
	{
		cout << "Error - Invalid socket\n";
		return false;
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
		return false;
	}

	// 3. ���Ŵ�⿭����
	if (listen(listenSocket, 5) == SOCKET_ERROR)
	{
		cout << "Error - Fail listen\n";
		// 6. ��������
		closesocket(listenSocket);
		// Winsock End
		WSACleanup();
		return false;
	}
	return true;
}

void Server::RunServer()
{
	iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
	for (int i = 0; i < MAX_USER; ++i)
	{
		//SOCKETINFO tmpClient;
		clients[i].prev_size = 0;
		clients[i].xPos = PLAYER_INIT_X_POS;
		clients[i].yPos = PLAYER_INIT_Y_POS;
		clients[i].zPos = PLAYER_INIT_Z_POS;
		clients[i].xLook = PLAYER_INIT_X_LOOK;
		clients[i].yLook = PLAYER_INIT_Y_LOOK;
		clients[i].zLook = PLAYER_INIT_Z_LOOK;
		clients[i].xUp = PLAYER_INIT_X_UP;
		clients[i].yUp = PLAYER_INIT_Y_UP;
		clients[i].zUp = PLAYER_INIT_Z_UP;
		clients[i].xRight = PLAYER_INIT_X_RIGHT;
		clients[i].yRight = PLAYER_INIT_Y_RIGHT;
		clients[i].zRight = PLAYER_INIT_Z_RIGHT;
		//clients.emplace_back(tmpClient);
		//printf("Create Client ID: %d, PrevSize: %d, xPos: %d, yPos: %d, zPos: %d, xDir: %d, yDir: %d, zDir: %d\n", i, clients[i].prev_size, clients[i].xPos, clients[i].yPos, clients[i].zPos, clients[i].xDir, clients[i].yDir, clients[i].zDir);
	}
	
	for (int i = 0; i < MAX_WORKER_THREAD; ++i)
		workerThreads.emplace_back(thread{ WorkerThread, (LPVOID)this });
	thread accpetThread{ AcceptThread, (LPVOID)this };
	accpetThread.join();
	for (auto &th : workerThreads)
		th.join();
}

void Server::AcceptThread(LPVOID arg)
{
	Server *pServer = static_cast<Server*>(arg);
	pServer->AcceptThreadFunc();
}

void Server::AcceptThreadFunc()
{
	SOCKADDR_IN clientAddr{};
	int addrLen = sizeof(SOCKADDR_IN);
	memset(&clientAddr, 0, addrLen);
	SOCKET clientSocket{};
	DWORD flags;

	while (1)
	{
		// clientSocket�� �񵿱������ ����� ���ؼ��� listenSocket�� �񵿱���̾�� �Ѵ�.
		clientSocket = accept(listenSocket, (struct sockaddr *)&clientAddr, &addrLen);
		if (clientSocket == INVALID_SOCKET)
		{
			cout << "Error - Accept Failure\n";
			return;
		}

		// id�� 0~5���� ����� ����
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
		
		///////////////////////////////////// Ŭ���̾�Ʈ �ʱ�ȭ ���� ���� ��ġ /////////////////////////////////////
		clients[new_id].socket = clientSocket;
		///////////////////////////////////// Ŭ���̾�Ʈ �ʱ�ȭ ���� ���� ��ġ /////////////////////////////////////
		ZeroMemory(&clients[new_id].over_ex.over, sizeof(clients[new_id].over_ex.over));
		
		flags = 0;

		CreateIoCompletionPort(reinterpret_cast<HANDLE>(clientSocket), iocp, new_id, 0);

		clients[new_id].in_use = true;

		SendAcessComplete(new_id);
		printf("%d Ŭ���̾�Ʈ ���� �Ϸ�\n", new_id);
		// ���� �����鿡�� ���� ������ ������ ���
		for (int i = 0; i < MAX_USER; ++i)
			if (true == clients[i].in_use)
				SendPutPlayer(i, new_id);

		// ó�� ������ ������ ���� ������ ���
		for (int i = 0; i < MAX_USER; ++i)
		{
			if (false == clients[i].in_use)
				continue;
			if (i == new_id)
				continue;
			SendPutPlayer(new_id, i);
		}

		RecvFunc(new_id);
	}

	// 6-2. ���� ��������
	closesocket(listenSocket);

	// Winsock End
	WSACleanup();
}


void Server::RecvFunc(char client)
{
	DWORD flags = 0;

	if (WSARecv(clients[client].socket, &clients[client].over_ex.dataBuffer, 1, NULL, &flags, &(clients[client].over_ex.over), 0))
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			cout << "Error - IO pending Failure\n";
			while (true);
		}
	}
	else {
		// �񵿱������ ���ư��� �ʰ� ��������� ���ư��ٴ� ����.
		// Ű�� ���ÿ� ���� �� �߻�(ex / UPŰ�� RIGHTŰ)
		cout << "Non Overlapped Recv return.\n";
		//while (true);
	}
}

void Server::WorkerThread(LPVOID arg)
{
	Server* pServer = static_cast<Server*>(arg);
	pServer->WorkerThreadFunc();
}

void Server::WorkerThreadFunc()
{
	while (true)
	{
		DWORD io_byte;
		ULONGLONG l_key;
		// �������� �����͸� �Ѱ����
		OVER_EX *over_ex;

		int is_error = GetQueuedCompletionStatus(iocp, &io_byte, &l_key, reinterpret_cast<LPWSAOVERLAPPED *>(&over_ex), INFINITE);

		if (0 == is_error)
		{
			int err_no = WSAGetLastError();
			if (64 == err_no)
			{
				ClientDisconnect(l_key);
				continue;
			}
			else
				err_display("GQCS : ");
		}

		if (0 == io_byte)
		{
			ClientDisconnect(l_key);
			continue;
		}

		char key = static_cast<char>(l_key);
		if (true == over_ex->is_recv)
		{
			// RECV ó��
			wcout << "Packet from Client: " << (int)key << "\n";
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
					ProcessPacket(key, clients[key].packet_buffer);
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

			RecvFunc(key);
		}
		else
		{
			// SEND ó��
			delete over_ex;
		}
	}
}

void Server::ProcessPacket(char client, char *packet)
{
	CS_PACKET_UP_KEY *p = reinterpret_cast<CS_PACKET_UP_KEY *>(packet);
	float x = clients[client].xPos;
	float y = clients[client].yPos;
	float z = clients[client].zPos;

	// 0���� ������, 1���� ��ŶŸ��
	switch (p->type)
	{
	case CS_UP_KEY:
		printf("Press UP Key ID: %d\n", client);
		y += 12.25f;
		break;
	case CS_DOWN_KEY:
		printf("Press DOWN Key ID: %d\n", client);
		y -= 12.25f;
		break;
	case CS_LEFT_KEY:
		printf("Press LEFT Key ID: %d\n", client);
		x -= 12.25f;
		break;
	case CS_RIGHT_KEY:
		printf("Press RIGHT Key ID: %d\n", client);
		x += 12.25f;
		break;
	default:
		wcout << L"���ǵ��� ���� ��Ŷ ���� ����!!\n";
		while (true);
	}
	clients[client].xPos = x;
	clients[client].yPos = y;
	clients[client].zPos = z;
	printf("Move Player ID: %d\tx: %f, y: %f, z: %f\n", client, x, y, z);

	SendMovePlayer(client);
}	

void Server::SendFunc(char client, void *packet)
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

void Server::SendAcessComplete(char client)
{
	SC_PACKET_ACCESS_COMPLETE packet;
	packet.myId = client;
	packet.size = sizeof(packet);
	packet.type = SC_ACCESS_COMPLETE;

	SendFunc(client, &packet);
}

void Server::SendPutPlayer(char toClient, char fromClient)
{
	SC_PACKET_PUT_PLAYER packet;
	packet.myId = fromClient;
	packet.size = sizeof(packet);
	packet.type = SC_PUT_PLAYER;
	packet.xPos = clients[fromClient].xPos;
	packet.yPos = clients[fromClient].yPos;
	packet.zPos = clients[fromClient].zPos;

	SendFunc(toClient, &packet);
}

void Server::SendMovePlayer(char client)
{
	SC_PACKET_MOVE_PLAYER packet;
	packet.id = client;
	packet.size = sizeof(packet);
	packet.type = SC_MOVE_PLAYER;
	packet.xPos = clients[client].xPos;
	packet.yPos = clients[client].yPos;
	packet.zPos = clients[client].zPos;

	SendFunc(client, &packet);
}

void Server::SendRemovePlayer(char toClient, char fromClient)
{
	SC_PACKET_REMOVE_PLAYER packet;
	packet.id = fromClient;
	packet.size = sizeof(packet);
	packet.type = SC_REMOVE_PLAYER;

	SendFunc(toClient, &packet);
}

void Server::ClientDisconnect(char client)
{
	for (int i = 0; i < MAX_USER; ++i)
	{
		if (false == clients[i].in_use)
			continue;
		if (i == client)
			continue;
		SendRemovePlayer(i, client);
	}
	closesocket(clients[client].socket);
	clients[client].in_use = false;
	printf("%d Ŭ���̾�Ʈ ���� ����\n", (int)client);
}

void Server::err_quit(const char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessage
	(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0,
		NULL
	);

	MessageBox(NULL, (LPCTSTR)lpMsgBuf, (LPCTSTR)msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	//exit(1);
}

void Server::err_display(const char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessage
	(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0,
		NULL
	);
	cout << msg << (char*)lpMsgBuf << endl;

	LocalFree(lpMsgBuf);
}