#pragma once

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h>
#include "..\Stdafx\Stdafx.h"
//#include "protocol.h"
#include "../../../../Server/FreezeBombServer/FreezeBombServer/protocol.h"
#include "../Singleton/Singleton.h"
#include <string>


constexpr int SERVER_PORT = 9000;
constexpr int BUF_SIZE = 1024;

class CGameFramework;


#ifdef _WITH_SERVER_
//�̱��� 


class Network : public Singleton<Network>
{
private:
	SOCKET	sock;
	int		myId;
	WSABUF	send_wsabuf;
	char 	send_buffer[BUF_SIZE];
	WSABUF	recv_wsabuf;
	char	recv_buffer[BUF_SIZE];
	char	packet_buffer[BUF_SIZE];
	DWORD	in_packet_size = 0;
	int		saved_packet_size = 0;


private:
	CS_PACKET_UP_KEY *pUp = NULL;
	CS_PACKET_DOWN_KEY *pDown = NULL;
	CS_PACKET_RIGHT_KEY *pRight = NULL;
	CS_PACKET_LEFT_KEY *pLeft = NULL;
	CS_PACKET_READY *pReady = NULL;
	CS_PACKET_UNREADY* pUnReady = NULL;
	CS_PACKET_REQUEST_START *pRequestStart = NULL;
	CS_PACKET_RELEASE_KEY *pReleaseKey = NULL;
	CS_PACKET_ANIMATION *pAnimation = NULL;
	CS_PACKET_NICKNAME *pNickName = NULL;
	CS_PACKET_CHATTING *pText = NULL;	
	CS_PACKET_OBJECT_COLLISION *pCollide = NULL;
	CS_PACKET_NOT_COLLISION *pNotCollide = NULL;
	CS_PACKET_PLAYER_COLLISION* pPlayerCollision = NULL;
	CS_PACKET_USE_ITEM *pItem = NULL;
	CS_PACKET_FREEZE *pFreeze = NULL;
	CS_PACKET_RELEASE_FREEZE *pReleaseFreeze = NULL;
	CS_PACKET_BOMB_EXPLOSION *pBomb = NULL;
	CS_PACKET_BOMBER_TOUCH* pTouch = NULL;
	CS_PACKET_GET_ITEM *pGetItem = NULL;
private:
	//ReadPacket���� ���� ��Ŷ���� CGameFramework�� �����ϱ� ���� ������
	CGameFramework* m_pGameClient{nullptr};
	const  char *   m_ServerIP{ nullptr };
public:
	void err_quit(const char *msg);
	void err_display(const char *msg);
	int GetMyID() { return myId; }
	void SetMyID(int id) { myId = id; }
	void SetServerIP(const char* ip) { m_ServerIP = ip; }
	const char* GetServerIP() { return m_ServerIP; }
public:
	bool m_connect{ false };
public:
	Network();
	~Network();
public:
	SOCKET getSock();
	bool connectToServer(HWND hWnd );

	//NetworkŬ������ CGameFramework�� ���ٰ����ϰ� �ϱ����� ���� �����͸� �����ְ� ��.
	void SetGameFrameworkPtr(HWND hWnd, CGameFramework* client);

	void ReadPacket();
	void SendPacket();
	void SendPacket(DWORD dataBytes);
public:
	void SendUpKey();
	void SendUpRightKey();
	void SendUpLeftKey();
	void SendDownKey();
	void SendDownRightKey();
	void SendDownLeftKey();
	void SendRightKey();
	void SendLeftKey();
	void SendReady(int matID);
	void SendNotReady();
	void SendReqStart();
	void SendReleaseKey();
	void SendAnimationState(char animNum);
	void SendNickName(char id,_TCHAR* name);
	void SendChattingText(char id,const _TCHAR* text);
	void SendSurroundingCollision(USHORT objID);
	void SendNotCollision();
	void SendPlayerCollision(unsigned char playerID);
	void SendGetItem(const string& itemIndex);
	void SendUseItem(int useItem, int targetID);
	void SendFreezeState();
	void SendReleaseFreezeState();
	void SendBombExplosion();
	void SendBomberTouch(char targetID);
public:
	CS_PACKET_REQUEST_START* GetRS() { return pRequestStart; }
	void SetNullRS() { pRequestStart = NULL; }
	/*SC_PACKET_ACCESS_COMPLETE* GetAC() { return pAC; }
	SC_PACKET_ACCESS_PLAYER* GetAP() { return pAP; }
	SC_PACKET_PUT_PLAYER* GetPP() {return pPP; }
	SC_PACKET_MOVE_PLAYER* GetMP() { return pMP; }
	SC_PACKET_USE_ITEM* GetUI() { return pUI; }
	SC_PACKET_ROLL_CHANGE* GetRC() { return pRC; }
	SC_PACKET_ROUND_END* GetRE() { return pRE; }
	SC_PACKET_REMOVE_PLAYER* GetRP() { return pRP; }
	SC_PACKET_COMPARE_TIME* GetCT() { return pCT; }
	SC_PACKET_ROUND_START* GetRS() { return pRS; }*/

};

#endif