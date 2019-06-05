#include "../Stdafx/Stdafx.h"
#include <algorithm>
#include "../Network/Network.h"
#include "Chatting.h"

#pragma comment(lib,"imm32.lib")
#include <imm.h>



#ifdef _WITH_DIRECT2D_

ChattingSystem::ChattingSystem()
{
}

ChattingSystem::~ChattingSystem()
{
	//cout << "Chattinng System �Ҹ�" << endl;
}

void ChattingSystem::Initialize(IDWriteFactory* writeFactory, ID2D1DeviceContext2* pd2dDeviceContext, IWICImagingFactory* pwicImagingFactory)
{
	HRESULT hResult;
	m_pdwChattingFont = new IDWriteTextFormat*[m_maxChatSentenceCount];
	m_pd2dbrChatText = new ID2D1SolidColorBrush*[m_maxChatSentenceCount];


	m_chat = new TCHAR*[m_maxChatSentenceCount];

	for (int i = 0; i < m_maxChatSentenceCount; ++i)
	{
		m_chat[i] = new TCHAR[256];

	}

	

	for (int i = 0; i < m_maxChatSentenceCount; ++i)
	{
		hResult = writeFactory->CreateTextFormat(L"���", nullptr, DWRITE_FONT_WEIGHT_DEMI_BOLD, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 30.0f, L"en-US", &m_pdwChattingFont[i]);
		hResult = m_pdwChattingFont[i]->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
		hResult = m_pdwChattingFont[i]->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
		hResult = writeFactory->CreateTextLayout(L"�ؽ�Ʈ ���̾ƿ�", 64, m_pdwChattingFont[i], 4096.0f, 4096.0f, &m_pdwChattingLayout);
		pd2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black, 1.0f), &m_pd2dbrChatText[i]);
		//pd2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Yellow, 1.0f), &m_pd2dbrChatText[i]);
	}

	/////////////////////////////////////////////////////��Ʈ ����

	///////////////////////////////////////////////////////////////////////////////////////////////////////
	IWICBitmapDecoder *pwicBitmapDecoder;
	hResult = pwicImagingFactory->CreateDecoderFromFilename(L"../Resource/Png/chatting.png", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);

	hResult = pd2dDeviceContext->CreateEffect(CLSID_D2D1BitmapSource, &m_pd2dfxBitmapSource);

	IWICBitmapFrameDecode *pwicFrameDecode;
	pwicBitmapDecoder->GetFrame(0, &pwicFrameDecode);	//GetFrame() : Retrieves the specified frame of the image.

	pwicImagingFactory->CreateFormatConverter(&m_pwicFormatConverter);

	m_pwicFormatConverter->Initialize(pwicFrameDecode, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0f, WICBitmapPaletteTypeCustom);

	m_pd2dfxBitmapSource->SetValue(D2D1_BITMAPSOURCE_PROP_WIC_BITMAP_SOURCE, m_pwicFormatConverter);
	D2D1_VECTOR_2F	vec{ 1.3f,0.5f };
	m_pd2dfxBitmapSource->SetValue(D2D1_BITMAPSOURCE_PROP_SCALE, vec);
}
TCHAR* ChattingSystem::StringToTCHAR(string& s)
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

string ChattingSystem::TCHARToString(const TCHAR* ptsz)
{
	size_t len = (int)wcslen((wchar_t*)ptsz);
	const size_t charlen = 2 * len + 1;

	char* psz = new char[charlen];
	wcstombs(psz, (wchar_t*)ptsz, charlen);

	string s = psz;
	delete[] psz;
	return s;
}
void ChattingSystem::ShowLobbyChatting(ID2D1DeviceContext2* pd2dDeviceContext)
{
	D2D1_RECT_F chatText{ 0,0,0,0 };
	if (IsChattingActive())
	{
		
		const TCHAR* t;
		t = m_wsChat.c_str();

		chatText = D2D1::RectF(680.0f, 645.0f, 1200.0f, 645.0f);
		pd2dDeviceContext->DrawTextW(t, (UINT32)wcslen(t), m_pdwChattingFont[0], &chatText, m_pd2dbrChatText[0]);


	}

	
	for (int i=0 ; i<m_nCurrentText;++i)
	{
		
		chatText = D2D1::RectF(680.0f, 580.0f - (i * 40), 1200.0f, 580.0f - (i * 40));
		pd2dDeviceContext->DrawTextW(m_arrText[i].first, m_arrText[i].second, m_pdwChattingFont[i], &chatText, m_pd2dbrChatText[i]);
		
	}


}


void ChattingSystem::ShowIngameChatting(ID2D1DeviceContext2* pd2dDeviceContext)
{

	if (IsChattingActive() && m_pd2dfxBitmapSource)
	{
		D2D_POINT_2F p{ -360.0f,660.0f };

		pd2dDeviceContext->DrawImage(m_pd2dfxBitmapSource, p);
		D2D1_RECT_F chatText{ 0,0,0,0 };
		const TCHAR* t;
		t = m_wsChat.c_str();

		chatText = D2D1::RectF(20.0f, 750.0f, 600.0f, 750.0f);
		pd2dDeviceContext->DrawTextW(t, (UINT32)wcslen(t), m_pdwChattingFont[0], &chatText, m_pd2dbrChatText[0]);
	}
}


void ChattingSystem::ProcessChatting(HWND hWnd, WPARAM wParam, LPARAM lParam, bool isInGame)
{

	//n_tcscpy_s()
	if (wParam == VK_RETURN)
	{
		if (m_wsChat.size() > 0)
		{
#ifdef _WITH_SERVER_
			const TCHAR* t;
			t = m_wsChat.c_str();

			Network::GetInstance()->SendChattingText((char)Network::GetInstance()->GetMyID(), t);

#endif
			m_wsChat.clear();
			m_wsChat.shrink_to_fit();
		}
	}


	//���� ������ Ȱ��ȭ ���� ���
	if (GetIMEMode() == IME_CMODE_ALPHANUMERIC)
	{
		if (wParam != VK_RETURN && wParam != VK_BACK)
		{
			if (isInGame) {
				if (m_wsChat.size() <= (size_t)SENTENCE_LENGTH_INGAME::ENG)
				{
					m_wsChat += (TCHAR)wParam;
				}
			}
			else
			{
				if (m_wsChat.size() <= (size_t)SENTENCE_LENGTH_LOBBY::ENG)
				{
					//m_wsChat += (TCHAR)wParam;
					m_wsChat += (TCHAR)wParam;
				}
			}

		}
	}
	else if (GetIMEMode() == IME_CMODE_NATIVE)
	{
		if (wParam != VK_RETURN)
		{
			if (isInGame)
			{
				if (m_wsChat.size() <= (size_t)SENTENCE_LENGTH_INGAME::KOR)
				{
					//m_wsChat += (TCHAR)wParam;
					ComposeHangeul(hWnd, wParam, lParam);
				}
			}
			else
			{
				if (m_wsChat.size() <= (size_t)SENTENCE_LENGTH_LOBBY::KOR)
				{
					//m_wsChat += (TCHAR)wParam;
					ComposeHangeul(hWnd, wParam, lParam);
				}
			}

		}

	}
	if (GetKeyState(VK_BACK) & 0x8000)
	{

		if (m_wsChat.size() > 0)
		{
			m_wsChat.pop_back();
		}
		if (GetIMEMode() == IME_CMODE_NATIVE)
		{
			SetIMEMode(hWnd, IME_CMODE_ALPHANUMERIC);
			SetIMEMode(hWnd, IME_CMODE_NATIVE);
		}
		m_composeCount = 0;
		//return;
	}


}

void ChattingSystem::Destroy()
{

	for (int i = 0; i < m_maxChatSentenceCount; ++i)
	{
		if (m_chat[i])
		{
			delete m_chat[i];
		}
	}
	if (m_chat)
	{
		delete[] m_chat;
	}

	m_wsChat.clear();
	m_wsChat.shrink_to_fit();
	for (int i = 0; i < m_maxChatSentenceCount; ++i)
	{
		if (m_pdwChattingFont[i]) m_pdwChattingFont[i]->Release();
		if (m_pd2dbrChatText[i]) m_pd2dbrChatText[i]->Release();
	}
	delete[]m_pdwChattingFont;
	m_pdwChattingFont = nullptr;
	delete[]m_pd2dbrChatText;
	m_pd2dbrChatText = nullptr;
	if (m_pd2dfxBitmapSource) m_pd2dfxBitmapSource->Release();
	if (m_pwicFormatConverter) m_pwicFormatConverter->Release();

}
#endif

void ChattingSystem::PushChattingText(char* chat)
{

	ZeroMemory(m_chat[m_nCurrentText], sizeof(256));
	int nLen = MultiByteToWideChar(CP_ACP, 0, chat, strlen(chat), NULL, NULL);
	MultiByteToWideChar(CP_ACP, 0, chat, strlen(chat), m_chat[m_nCurrentText], nLen);
	m_arrText[m_nCurrentText] = make_pair(m_chat[m_nCurrentText], (UINT32)nLen);
	m_nCurrentText = (++m_nCurrentText) % m_maxChatSentenceCount;

}

//���α׷� ������ �ѿ� ��ȯ
void ChattingSystem::SetIMEMode(HWND hWnd, bool bHanMode)
{
	//�ش� �������� Input Context �� �������� �Լ�.
	//���ϰ����� Input Context�� �ڵ��� ��ȯ�Ѵ�.
	HIMC hIMC = ImmGetContext(hWnd);
	DWORD dwSent;
	DWORD dwTemp;

	//hIMC => ������ �˰���� input context�� �ڵ�
	//lpfwdConversion => IME Conversion Mode ��
	//lpfwdSentence => Sentence mode ��
	ImmGetConversionStatus(hIMC, &m_conv, &dwSent);

	dwTemp = m_conv & ~IME_CMODE_LANGUAGE;

	//���¸� �ٲ۴�
	if (bHanMode)
		dwTemp |= IME_CMODE_NATIVE;	//�ѱ�
	else
		dwTemp |= IME_CMODE_ALPHANUMERIC;	// ����

	m_conv = dwTemp;

	//hIMC - input context �ڵ�, NULL�̸� ���� active ������ context�� ����

	ImmSetConversionStatus(hIMC, m_conv, dwSent);

	//Input Context�� ������ �ϰ�, Context�� �Ҵ�� �޸𸮸� unlock �Ѵ�.
	//ImmGetContext()�� ����ߴٸ�, �ݵ�� ImmReleaseContext()�� ������־�� �Ѵ�.
	ImmReleaseContext(hWnd, hIMC);


}


bool ChattingSystem::ComposeHangeul(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	HIMC hImc = ImmGetContext(hWnd);

	if (hImc == nullptr)
		return false;


	int nLength = 0;
	TCHAR wszComp[4] = { 0, };

	//IME ���� ������ �Ϸ�
	if (lParam & GCS_RESULTSTR)
	{
		nLength = ImmGetCompositionStringW(hImc, GCS_RESULTSTR, nullptr, 0);
		if (nLength > 0)
		{
			ImmGetCompositionStringW(hImc, GCS_RESULTSTR, wszComp, nLength);

			for (int i = 0; i < nLength; ++i)
			{
				if (wszComp[i] != 0)
				{

					//���յ� ���� �������� �ϱ�
					wstring s(wszComp);
					if (m_wsChat.size() > 0 && m_composeCount > 0)
					{
						m_wsChat.pop_back();
					}
					m_wsChat += s;

					//	m_iter = m_wsChat.rbegin();

					m_composeCount = 0;
					//return wszComp;
				}
			}
		}
	}
	else if (lParam & GCS_COMPSTR)
	{
		nLength = ImmGetCompositionStringW(hImc, GCS_COMPSTR, nullptr, 0);


		if (nLength > 0)
		{
			ImmGetCompositionStringW(hImc, GCS_COMPSTR, wszComp, nLength);


			for (int i = 0; i < nLength; ++i)
			{
				if (wszComp[i] != 0)
				{

					wstring s(wszComp);
					if (m_composeCount > 0 && m_wsChat.size() > 0)
					{
						m_wsChat.pop_back();
						m_composeCount--;
					}
					m_wsChat += s;

					m_composeCount++;
					//���� ���� ���ڰ� �������� �ڵ� ����


				}
			}
		}
	}

	//IME�ڵ� ����
	ImmReleaseContext(hWnd, hImc);
	return true;

}

