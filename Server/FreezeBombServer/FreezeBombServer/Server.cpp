#include "Server.h"


Server::Server()
{
	clientCount = 0;
	hostId = -1;
	//clients.reserve(MAX_USER);
	workerThreads.reserve(MAX_WORKER_THREAD);
}


Server::~Server()
{
	//clients.clear();
	if (heightMap)
		delete heightMap;
}

bool Server::InitServer()
{
	clientCount = 0;
	hostId = -1;
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
	gameTimer.Start();
	iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
	for (int i = 0; i < MAX_USER; ++i)
	{
		//SOCKETINFO tmpClient;
		clients[i].prev_size = 0;
		clients[i].pos.x = PLAYER_INIT_X_POS;
		clients[i].pos.y = PLAYER_INIT_Y_POS;
		clients[i].pos.z = PLAYER_INIT_Z_POS;
		clients[i].look.x = PLAYER_INIT_X_LOOK;
		clients[i].look.y = PLAYER_INIT_Y_LOOK;
		clients[i].look.z = PLAYER_INIT_Z_LOOK;
		clients[i].up.x = PLAYER_INIT_X_UP;
		clients[i].up.y = PLAYER_INIT_Y_UP;
		clients[i].up.z = PLAYER_INIT_Z_UP;
		clients[i].right.x = PLAYER_INIT_X_RIGHT;
		clients[i].right.y = PLAYER_INIT_Y_RIGHT;
		clients[i].right.z = PLAYER_INIT_Z_RIGHT;
		//clients.emplace_back(tmpClient);
		//printf("Create Client ID: %d, PrevSize: %d, xPos: %d, yPos: %d, zPos: %d, xDir: %d, yDir: %d, zDir: %d\n", i, clients[i].prev_size, clients[i].xPos, clients[i].yPos, clients[i].zPos, clients[i].xDir, clients[i].yDir, clients[i].zDir);
	}
	heightMap = new CHeightMapImage("../../../Client/FreezeBomb/Resource/Textures/Terrain/Terrain.raw", 256, 256, XMFLOAT3(2.0f, 1.0f, 2.0f));
	
	gameTimer.Tick(60.0f);
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
		if (-1 == hostId)
		{
			hostId = new_id;
			printf("���� ������ %d�Դϴ�.\n", hostId);
		}
		///////////////////////////////////// Ŭ���̾�Ʈ �ʱ�ȭ ���� ���� ��ġ /////////////////////////////////////
		ZeroMemory(&clients[new_id].over_ex.over, sizeof(clients[new_id].over_ex.over));
		
		flags = 0;

		CreateIoCompletionPort(reinterpret_cast<HANDLE>(clientSocket), iocp, new_id, 0);

		clients[new_id].in_use = true;
		clients[new_id].velocity = XMFLOAT3(0.0f, 0.0f, 1.0f);

		SendAccessComplete(new_id);
		// ���� �����鿡�� ���� ������ ������ ���
		for (int i = 0; i < MAX_USER; ++i)
			if (true == clients[i].in_use)
				SendAccessPlayer(i, new_id);

		// ó�� ������ ������ ���� ������ ���
		for (int i = 0; i < MAX_USER; ++i)
		{
			if (false == clients[i].in_use)
				continue;
			if (i == new_id)
				continue;
			SendAccessPlayer(new_id, i);
		}
		++clientCount;
		printf("%d Ŭ���̾�Ʈ ���� �Ϸ�, ���� Ŭ���̾�Ʈ ��: %d\n", new_id, clientCount);
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
			//wcout << "Packet from Client: " << (int)key << "\n";
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
	float x = clients[client].pos.x;
	float y = clients[client].pos.y;
	float z = clients[client].pos.z;

	DWORD tmpDir;

	// 0���� ������, 1���� ��ŶŸ��
	switch (packet[1])
	{
	case CS_UP_KEY:
	case CS_DOWN_KEY:
	case CS_LEFT_KEY:
	case CS_RIGHT_KEY:
		SetDirection(client, packet[1]);
		UpdateClientPos(client, gameTimer.GetTimeElapsed());


		//printf("Move Player ID: %d\tx: %f, y: %f, z: %f\n", client, x, y, z);
		for (int i = 0; i < MAX_USER; ++i)
		{
			if (clients[i].in_use == true)
			{
				SendMovePlayer(i,client);
				//�ѹ� MovePacket�� ������ ���� 
				//pitch,yaw,roll�� �ٽ� 0���� �ٲ������.
				//�׷��� ������� ��۹�� ���Ե�.
				SetPitchYawRollZero(i);
				
				//Idle �������� ���ϰ� �Ϸ��� 
				//SetVelocityZero(i);
			}
		}
		break;
	case CS_READY:
		printf("��ü Ŭ�� ��: %d\n", clientCount);
		// Ŭ�� ���ʹ����� F5���������� CS_READY ��Ŷ�� ���ƿ´ٸ� ++readyCount�� clientCount���� �����ϰ� �ǰ� 
		// �Ʒ� CS_REQUEST_START�ȿ� if(clientCount<= readyCount) ������ ���� �ʴ� ���� �߻�
		printf("Ready�� Ŭ�� ��: %d\n", ++readyCount);

		clients[client].isReady = true;
		
		clients[client].matID = packet[2];	// matID
		printf("Recv matID : %d\n", clients[client].matID);
		break;
	case CS_REQUEST_START:
		if (clientCount <= readyCount)
		{
		
			for(int i=0; i< MAX_USER ;++i)
			{
				if(true == clients[i].in_use)
				{
					SetClient_Initialize(i);
					SendRoundStart(i);
					for (int j = 0; j < MAX_USER; ++j)
					{

						if (true == clients[j].in_use)
						{
							SendPutPlayer(i, j);
						}
					}
				}
			}
			//for (int i = 0; i < MAX_USER; ++i)
			//{
			//	if (true == clients[i].in_use)
			//	{
			//		//�����ϴ� Ŭ���̾�Ʈ ���� look,right,up���͸� �ʱ�ȭ �������.
			//		SetClient_Initialize(i);
			//		
			//		for (int j = 0; j < MAX_USER; ++j)
			//		{
			//			if (true == clients[j].in_use)
			//			{

			//				SendPutPlayer(i, j);
			//				//SendPutPlayer(j, i);
			//			}
			//		}
			//		SendRoundStart(i);
			//	}
			//}
			printf("Round Start\n");
		}
		else
		{
			//�� �κ� READYCOUNT ����
			for (int i = 0; i < MAX_USER; ++i)
			{
				if (true == clients[i].in_use)
					SendPleaseReady(i);
			}
			printf("Please Ready\n");
		}
		break;
	default:
		wcout << L"���ǵ��� ���� ��Ŷ ���� ����!!\n";
		while (true);
	}
	
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

void Server::SendAccessComplete(char client)
{
	SC_PACKET_ACCESS_COMPLETE packet;
	packet.myId = client;
	packet.hostId = hostId;
	packet.size = sizeof(packet);
	packet.type = SC_ACCESS_COMPLETE;

	SendFunc(client, &packet);
}

void Server::SendAccessPlayer(char toClient, char fromClient)
{
	SC_PACKET_ACCESS_PLAYER packet;
	packet.id = fromClient;
	packet.size = sizeof(packet);
	packet.type = SC_ACCESS_PLAYER;

	SendFunc(toClient, &packet);
}

void Server::SendPutPlayer(char toClient, char fromClient)
{
	SC_PACKET_PUT_PLAYER packet;
	packet.id = fromClient;
	packet.size = sizeof(packet);
	packet.type = SC_PUT_PLAYER;
	packet.xPos = clients[fromClient].pos.x;
	packet.yPos = clients[fromClient].pos.y;
	packet.zPos = clients[fromClient].pos.z;
	packet.xLook = clients[fromClient].look.x;
	packet.yLook = clients[fromClient].look.y;
	packet.zLook = clients[fromClient].look.z;
	packet.xUp = clients[fromClient].up.x;
	packet.yUp = clients[fromClient].up.y;
	packet.zUp = clients[fromClient].up.z;
	packet.xRight = clients[fromClient].right.x;
	packet.yRight = clients[fromClient].right.y;
	packet.zRight = clients[fromClient].right.z;
	packet.matID = clients[fromClient].matID;
	printf("Send matID : %d\n", packet.matID);
	SendFunc(toClient, &packet);
}

void Server::SendRoundStart(char client)
{
	SC_PACKET_ROUND_START packet;
	packet.size = sizeof(packet);
	packet.clientCount = clientCount;
	packet.type = SC_ROUND_START;

	SendFunc(client, &packet);
}

void Server::SendPleaseReady(char client)
{
	SC_PACKET_PLEASE_READY packet;
	packet.size = sizeof(packet);
	packet.type = SC_PLEASE_READY;

	SendFunc(client, &packet);

}


//���� �÷��̾� ��Ŷ�� ����� �������� ��û�� Ŭ���̾�Ʈ���Ը�
//�����ϴ� �������� �̺κ� �ٲ��� �ҵ� - ����

//TO ,Object
void Server::SendMovePlayer(char to,char client)
{
	SC_PACKET_MOVE_PLAYER packet;
	packet.id = client;
	packet.size = sizeof(packet);
	packet.type = SC_MOVE_PLAYER;

	packet.xPos = clients[client].pos.x;
	packet.yPos = clients[client].pos.y;
	packet.zPos = clients[client].pos.z;
	//�÷��̾��� ���� ���� ���� ,look,right,up
	packet.xLook = clients[client].look.x;
	packet.yLook = clients[client].look.y;
	packet.zLook = clients[client].look.z;
	packet.xRight= clients[client].right.x;
	packet.yRight= clients[client].right.y;
	packet.zRight= clients[client].right.z;
	packet.xUp = clients[client].up.x;
	packet.yUp = clients[client].up.y;
	packet.zUp = clients[client].up.z;

	//�÷��̾� ���� ���� ȸ������
	packet.pitch = clients[client].pitch;
	packet.yaw = clients[client].yaw;
	packet.roll = clients[client].roll;

	packet.fVelocity = clients[client].fVelocity;

	SendFunc(to, &packet);
}

void Server::SendRemovePlayer(char toClient, char fromClient)
{
	SC_PACKET_REMOVE_PLAYER packet;
	packet.id = fromClient;
	packet.hostId = hostId;
	packet.size = sizeof(packet);
	packet.type = SC_REMOVE_PLAYER;

	SendFunc(toClient, &packet);
}

void Server::ClientDisconnect(char client)
{
	
	clients[client].in_use = false;
	if (clients[client].isReady)
	{
		readyCount--;
		clients[client].isReady = false;
	}
	if (hostId == client)
	{
		for (int i = 0; i < MAX_USER; ++i)
		{
			if (true == clients[i].in_use)
			{
				hostId = i;
				printf("���� ������ %d�Դϴ�.\n", hostId);
				break;
			}
		}
	}
	for (int i = 0; i < MAX_USER; ++i)
	{
		if (false == clients[i].in_use)
			continue;
		if (i == client)
			continue;
		SendRemovePlayer(i, client);
	}
	closesocket(clients[client].socket);
	clientCount--;
	printf("%d Ŭ���̾�Ʈ ���� ����, ���� Ŭ���̾�Ʈ ��: %d\n", (int)client, clientCount);
}

void Server::SetDirection(char client, int key)
{
	DWORD tmpDir = 0;
	switch (key)
	{
	case CS_UP_KEY:
		tmpDir |= DIR_FORWARD;
		break;
	case CS_DOWN_KEY:
		tmpDir |= DIR_BACKWARD;
		break;
	case CS_RIGHT_KEY:
		tmpDir |= DIR_RIGHT;
		break;
	case CS_LEFT_KEY:
		tmpDir |= DIR_LEFT;
		break;
	}
	clients[client].direction = tmpDir;
}

void Server::SetVelocityZero(char client)
{
	clients[client].fVelocity = 0.0f;
}
void Server::SetClient_Initialize(char client)
{
	clients[client].look = XMFLOAT3(0.0f, 0.0f, 1.0f);
	clients[client].right = XMFLOAT3(1.0f, 0.0f, 0.0f);
	clients[client].up = XMFLOAT3(0.0f, 1.0f, 0.0f);
}

void Server::SetPitchYawRollZero(char client)
{
	if (clients[client].pitch > 0.0f)
		clients[client].pitch = 0.0f;
	if (clients[client].yaw > 0.0f)
		clients[client].yaw = 0.0f;
	if (clients[client].roll > 0.0f)
		clients[client].roll = 0.0f;
	
}
void Server::UpdateClientPos(char client, float fTimeElapsed)
{

	if (clients[client].direction == DIR_FORWARD) {
		clients[client].velocity = Vector3::Add(clients[client].velocity, clients[client].look, 1.0f);
	}
	if (clients[client].direction == DIR_BACKWARD)
	{
		clients[client].velocity = Vector3::Add(clients[client].velocity, clients[client].look , -1.0f);
	}
	if (clients[client].direction == DIR_LEFT || clients[client].direction == DIR_RIGHT)
	{
		RotateClientAxisY(client, fTimeElapsed);
	}
	clients[client].velocity = Vector3::Add(clients[client].velocity, gravity);
	float fLength = sqrtf(clients[client].velocity.x * clients[client].velocity.x + clients[client].velocity.z * clients[client].velocity.z);
	if (fLength > MAX_VELOCITY_XZ)
	{
		clients[client].velocity.x *= (MAX_VELOCITY_XZ / fLength);
		clients[client].velocity.z *= (MAX_VELOCITY_XZ / fLength);
	}

	float fLengthY = sqrtf(clients[client].velocity.y * clients[client].velocity.y);


	XMFLOAT3 xmf3Velocity = Vector3::ScalarProduct(clients[client].velocity, fTimeElapsed, false);

	clients[client].pos = Vector3::Add(clients[client].pos, clients[client].velocity);

	//clients[client].xmf3Position = xmf3Velocity;

	//clients[client].velocity = clients[client].velocity;

	ProcessClientHeight(client);

	ProcessFriction(client, fLength);

	//�ӵ��� Ŭ�󿡰� �����־� Ŭ�󿡼� �⺻���� rUn,Backward,�ִϸ��̼��� �����ϰ� �ϱ� ����.
	clients[client].fVelocity = fLength;

}

void Server::RotateClientAxisY(char client, float fTimeElapsed)
{
	XMFLOAT3& xmf3Look = clients[client].look;
	XMFLOAT3& xmf3Right = clients[client].right;
	XMFLOAT3& xmf3Up = clients[client].up;

	clients[client].lastLookVector = xmf3Look;
	clients[client].lastRightVector = xmf3Right;
	clients[client].lastUpVector = xmf3Up;

	if (clients[client].direction & DIR_RIGHT)
	{
		float fDotProduct = Vector3::DotProduct(xmf3Look, xmf3Right);

		float fAngle = ::IsEqual(fDotProduct, 1.0f) ? 0.0f : ((fDotProduct > 1.0f) ? XMConvertToDegrees(acos(fDotProduct)) : 90.0f);

		XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&xmf3Up), XMConvertToRadians(fAngle*fTimeElapsed));
		xmf3Look = Vector3::TransformNormal(xmf3Look, xmmtxRotate);
		xmf3Right = Vector3::TransformNormal(xmf3Right, xmmtxRotate);

		float cxDelta = xmf3Right.x - clients[client].lastRightVector.x;
		float cyDelta = xmf3Up.y - clients[client].lastUpVector.y;
		float czDelta = xmf3Look.z - clients[client].lastLookVector.z;


		RotateModel(client, 0.0f, fAngle*fTimeElapsed, 0.0f);
	}
	else if (clients[client].direction & DIR_LEFT)
	{
		float fDotProduct = Vector3::DotProduct(xmf3Look, xmf3Right);

		float fAngle = ::IsEqual(fDotProduct, 1.0f) ? 0.0f : ((fDotProduct > 1.0f) ? XMConvertToDegrees(acos(fDotProduct)) : 90.0f);

		XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&xmf3Up), XMConvertToRadians(-(fAngle*fTimeElapsed)));
		xmf3Look = Vector3::TransformNormal(xmf3Look, xmmtxRotate);
		xmf3Right = Vector3::TransformNormal(xmf3Right, xmmtxRotate);

		float czDelta = xmf3Look.z - clients[client].lastLookVector.z;

		RotateModel(client, 0.0f, -fAngle * fTimeElapsed, 0.0f);
	}

	xmf3Look = Vector3::Normalize(xmf3Look);
	xmf3Right = Vector3::CrossProduct(xmf3Up, xmf3Look, true);
	xmf3Up = Vector3::CrossProduct(xmf3Look, xmf3Right, true);



}

void Server::RotateModel(char client, float x, float y, float z)
{
	if (x != 0.0f)
	{
		clients[client].pitch += x;
		if (clients[client].pitch > +89.0f) { x -= (clients[client].pitch - 89.0f); clients[client].pitch = +89.0f; }
		if (clients[client].pitch < -89.0f) { x -= (clients[client].pitch + 89.0f); clients[client].pitch = -89.0f; }
	}
	if (y != 0.0f)
	{
		clients[client].yaw += y;
		if (clients[client].yaw > 360.0f) clients[client].yaw -= 360.0f;
		if (clients[client].yaw < 0.0f) clients[client].yaw += 360.0f;
	}
	if (z != 0.0f)
	{
		clients[client].roll += z;
		if (clients[client].roll > +20.0f) { z -= (clients[client].roll - 20.0f); clients[client].roll = +20.0f; }
		if (clients[client].roll < -20.0f) { z -= (clients[client].roll + 20.0f); clients[client].roll = -20.0f; }
	}
}

void Server::ProcessClientHeight(char client)
{
	int z = (int)(clients[client].pos.z / heightMap->GetScale().z);
	bool bReverseQuad = ((z % 2) != 0);

	float fHeight = heightMap->GetHeight(clients[client].pos.x, clients[client].pos.z, bReverseQuad);
	if (clients[client].pos.y < fHeight)
	{
		clients[client].velocity.y = 0.0f;
		clients[client].pos.y = fHeight;
	}
}

void Server::ProcessFriction(char client, float& fLength)
{
	fLength = Vector3::Length(clients[client].velocity);
	float fDeclaration = (FRICTION * gameTimer.GetTimeElapsed());

	if (fDeclaration > fLength)
	{
		fDeclaration = fLength;
		clients[client].velocity = Vector3::Add(clients[client].velocity, Vector3::ScalarProduct(clients[client].velocity, -fDeclaration, true));
	}
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