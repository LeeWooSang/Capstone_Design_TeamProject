#include "../Stdafx/Stdafx.h"
#include "IPInputSystem.h"
#include "../Shader/BillboardShader/UIShader/LoginShader/IPShader.h"
#include "../Network/Network.h"
#include "../Direct2D/Direct2D.h"

#pragma comment(lib,"imm32.lib")
#include <imm.h>

#ifdef _WITH_SERVER_
#ifdef _WITH_DIRECT2D_

CIPInputSystem::CIPInputSystem()
{
}

CIPInputSystem::~CIPInputSystem()
{
	m_wsIP.clear();
}

void CIPInputSystem::Initialize(IDWriteTextFormat* pFont, IDWriteTextLayout* pTextLayout, ID2D1SolidColorBrush* pFontColor, ID2D1DeviceContext2* pContext)
{
	m_pFont = pFont;
	m_pTextLayout = pTextLayout;
	m_pFontColor = pFontColor;
	m_pd2dDeviceContext = pContext;
}

size_t CIPInputSystem::ProcessIPInput()
{
		if (m_wsIP.size() < IP_LENGTH)
		{
			if (GetAsyncKeyState(DIGIT_0) & 0x0001 || GetAsyncKeyState(VK_NUMPAD0) & 0x0001)
				m_wsIP += '0';
			if (GetAsyncKeyState(DIGIT_1) & 0x0001 || GetAsyncKeyState(VK_NUMPAD1) & 0x0001)
				m_wsIP += '1';
			if (GetAsyncKeyState(DIGIT_2) & 0x0001 || GetAsyncKeyState(VK_NUMPAD2) & 0x0001)
				m_wsIP += '2';
			if (GetAsyncKeyState(DIGIT_3) & 0x0001 || GetAsyncKeyState(VK_NUMPAD3) & 0x0001)
				m_wsIP += '3';
			if (GetAsyncKeyState(DIGIT_4) & 0x0001 || GetAsyncKeyState(VK_NUMPAD4) & 0x0001)
				m_wsIP += '4';
			if (GetAsyncKeyState(DIGIT_5) & 0x0001 || GetAsyncKeyState(VK_NUMPAD5) & 0x0001)
				m_wsIP += '5';
			if (GetAsyncKeyState(DIGIT_6) & 0x0001 || GetAsyncKeyState(VK_NUMPAD6) & 0x0001)
				m_wsIP += '6';
			if (GetAsyncKeyState(DIGIT_7) & 0x0001 || GetAsyncKeyState(VK_NUMPAD7) & 0x0001)
				m_wsIP += '7';
			if (GetAsyncKeyState(DIGIT_8) & 0x0001 || GetAsyncKeyState(VK_NUMPAD8) & 0x0001)
				m_wsIP += '8';
			if (GetAsyncKeyState(DIGIT_9) & 0x0001 || GetAsyncKeyState(VK_NUMPAD9) & 0x0001)
				m_wsIP += '9';
			if (GetAsyncKeyState(VK_OEM_PERIOD) & 0x0001)
				m_wsIP += '.';
		}

		if (GetAsyncKeyState(VK_BACK) & 0x0001)
		{
			if (m_wsIP.size() > 0)
			{
				m_wsIP.pop_back();
			}
		}

		if(GetAsyncKeyState(VK_RETURN) & 0x0001)
		{
			if(m_wsIP.size() > 0 )
			{
				Network::GetInstance()->SetServerIP( m_wsIP.c_str());
				Network::GetInstance()->SetConnectState(Network::CONNECT_STATE::TRY);
			}
		}
	
	return m_wsIP.size();
}

TCHAR* CIPInputSystem::StringToTCHAR(const string& s)
{
	tstring tstr;
	const char* all = s.c_str();
	size_t len = strlen(all) + 1;

	wchar_t* t = new wchar_t[len];

	if (t == nullptr)
		throw bad_alloc();

	mbstowcs(t, all, len);

	return (TCHAR*)t;
}

void CIPInputSystem::ShowIPInput()
{
	const wstring& wstr = StringToTCHAR(m_wsIP);

	UINT originX = 1280;
	UINT originY = 780;

	D2D1_RECT_F pos{0,};

	pos.left = (480 * FRAME_BUFFER_WIDTH) / originX;
	pos.right = (750 * FRAME_BUFFER_WIDTH) / originX;
	pos.top = (650 * FRAME_BUFFER_HEIGHT) / originY;
	pos.bottom = (650 * FRAME_BUFFER_HEIGHT) / originY;

	CDirect2D::GetInstance()->Render("메이플", "검은색", wstr, pos);
}

string CIPInputSystem::TCHARToString(const TCHAR* ptsz)
{
	size_t len = (int)wcslen((wchar_t*)ptsz);
	const size_t charlen = 2 * len + 1;

	char* psz = new char[charlen];
	wcstombs(psz, (wchar_t*)ptsz, charlen);

	string s = psz;
	delete[] psz;
	return s;
}

void CIPInputSystem::Destroy()
{
	m_wsIP.clear();
	m_wsIP.shrink_to_fit();	
}

#endif
#endif