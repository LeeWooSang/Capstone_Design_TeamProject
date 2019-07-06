#include "../Stdafx/Stdafx.h"
#include <algorithm>
#include "../Network/Network.h"
#include "Chatting.h"

#pragma comment(lib,"imm32.lib")
#include <imm.h>

#define NUMBER DIGIT_0 | DIGIT_1 | DIGIT_2 | DIGIT_3 | DIGIT_4
#ifdef _WITH_DIRECT2D_
ChattingSystem::ChattingSystem()
{
}

ChattingSystem::~ChattingSystem()
{
}

void ChattingSystem::Initialize(IDWriteFactory* writeFactory, ID2D1DeviceContext2* pd2dDeviceContext, IWICImagingFactory* pwicImagingFactory, ID2D1SolidColorBrush* color)
{
	HRESULT hResult;
	m_chat = new TCHAR*[m_maxChatSentenceCount];

	for (int i = 0; i < m_maxChatSentenceCount; ++i)
	{
		m_chat[i] = new TCHAR[256];
	}

	hResult = writeFactory->CreateTextFormat(L"���", nullptr, DWRITE_FONT_WEIGHT_DEMI_BOLD, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 30.0f, L"en-US", &m_pChattingFont);
	hResult = m_pChattingFont->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
	hResult = m_pChattingFont->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
	hResult = writeFactory->CreateTextLayout(L"�ؽ�Ʈ ���̾ƿ�", 64, m_pChattingFont, 4096.0f, 4096.0f, &m_pChattingLayout);
	m_pChattingFontColor = color;

	for (int i = 0; i < m_maxChatSentenceCount; ++i)
	{
		//if (i == 0)
		//{
		//	hResult = writeFactory->CreateTextFormat(L"���", nullptr, DWRITE_FONT_WEIGHT_DEMI_BOLD, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 30.0f, L"en-US", &m_pChattingFont);

		//}
		//else
		//{
		//	hResult = writeFactory->CreateTextFormat(L"���", nullptr, DWRITE_FONT_WEIGHT_DEMI_BOLD, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 20.0f, L"en-US", &m_pChattingFont);
		//}
		//hResult = m_pChattingFont->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
		//hResult = m_pChattingFont->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
		//hResult = writeFactory->CreateTextLayout(L"�ؽ�Ʈ ���̾ƿ�", 64, m_pChattingFont, 4096.0f, 4096.0f, &m_pChattingLayout);
		//pd2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Coral, 1.0f), &m_pd2dbrChatText[i]);
		//pd2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Yellow, 1.0f), &m_pd2dbrChatText[i]);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////
	IWICBitmapDecoder *pwicBitmapDecoder;
	hResult = pwicImagingFactory->CreateDecoderFromFilename(L"../Resource/Png/chatting.png", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);

	hResult = pd2dDeviceContext->CreateEffect(CLSID_D2D1BitmapSource, &m_pd2dfxBitmapSource);

	IWICBitmapFrameDecode *pwicFrameDecode;
	pwicBitmapDecoder->GetFrame(0, &pwicFrameDecode);	//GetFrame() : Retrieves the specified frame of the image.

	pwicImagingFactory->CreateFormatConverter(&m_pwicFormatConverter);

	m_pwicFormatConverter->Initialize(pwicFrameDecode, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0f, WICBitmapPaletteTypeCustom);

	m_pd2dfxBitmapSource->SetValue(D2D1_BITMAPSOURCE_PROP_WIC_BITMAP_SOURCE, m_pwicFormatConverter);
	D2D1_VECTOR_2F vec{ 1.3f,0.5f };
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
		pd2dDeviceContext->DrawTextW(t, (UINT32)wcslen(t), m_pChattingFont, &chatText, m_pChattingFontColor);
	}

	for (int i = 1; i < m_dequeText.size()+1; ++i) 
	{
		chatText = D2D1::RectF(680.0f, 600.0f - (i * 24), 1200.0f, 600.0f - (i * 24));
		pd2dDeviceContext->DrawTextW(m_dequeText[i-1].first, m_dequeText[i-1].second, m_pChattingFont, &chatText, m_pChattingFontColor);
	}
}


void ChattingSystem::ShowIngameChatting(ID2D1DeviceContext2* pd2dDeviceContext,float fTimeElapsed)
{
	
	if (IsChattingActive() && m_pd2dfxBitmapSource)
	{
		D2D_POINT_2F p{ -360.0f,660.0f };

		pd2dDeviceContext->DrawImage(m_pd2dfxBitmapSource, p);
		D2D1_RECT_F chatText{ 0,0,0,0 };
		const TCHAR* t;
		t = m_wsChat.c_str();

		chatText = D2D1::RectF(20.0f, 750.0f, 600.0f, 750.0f);
		pd2dDeviceContext->DrawTextW(t, (UINT32)wcslen(t), m_pChattingFont, &chatText, m_pChattingFontColor);

		m_showTime = 0.0f;
	}
	else
	{
		m_showTime += fTimeElapsed;
	}
	if (m_showTime <= 5.0f) 
	{
		D2D1_RECT_F chatText{ 0,0,0,0 };
		for (int i = 1; i < m_dequeText.size() + 1; ++i)
		{
			chatText = D2D1::RectF(20.0f, 705.0f - (i * 24), 600.0f, 705.0f - (i * 24));
			pd2dDeviceContext->DrawTextW(m_dequeText[i - 1].first, m_dequeText[i - 1].second, m_pChattingFont, &chatText, m_pChattingFontColor);
		}
	}
}

void ChattingSystem::ProcessSpecialCharacter(WPARAM wParam)
{
	//n_tcscpy_s()
	if(DIGIT_0 <= wParam && wParam <= DIGIT_9)
	{
		if (m_wsChat.size() <= (size_t)18)
		{
			m_wsChat += (TCHAR)wParam;
		}
		
	}
	switch(wParam)
	{
	case '?':
	{
		
		if(m_wsChat.size() <= (size_t)15)
		{
			m_wsChat.append(L"?");
		}
		break;
	}
	case '!':
	{
		if(m_wsChat.size() <= (size_t)15)
		{
			m_wsChat.append(L"!");
		}
		break;
	}
	case '@':
	{
		if(m_wsChat.size() <= (size_t)14)
		{
			m_wsChat.append(L"@");
		}
		break;
	}
	case '_':
	{
		if(m_wsChat.size() <= (size_t)15)
		{
			m_wsChat.append(L"_");
		}
		break;
	}
	case '-':
	{
		if(m_wsChat.size() <= (size_t)15)
		{
			m_wsChat.append(L"-");
		}
		break;
	}
	case '%':
	{
		if(m_wsChat.size() <= (size_t)14)
		{
			m_wsChat.append(L"%");
		}
		break;
	}
	case '^':
	{
		if(m_wsChat.size() <= (size_t)15)
		{
			m_wsChat.append(L"^");
		}
		break;
	}
	case '(':
	{
		if(m_wsChat.size() <= (size_t)15)
		{
			m_wsChat.append(L"(");
		}
		break;
	}
	case ')':
	{
		if(m_wsChat.size() <= (size_t)15)
		{
			m_wsChat.append(L")");
		}
		break;
	}
	case '$':
	{
		if(m_wsChat.size() <= (size_t)14)
		{
			m_wsChat.append(L"$");
		}
		break;
	}
	case '#':
	{
		if(m_wsChat.size() <= (size_t)14)
		{
			m_wsChat.append(L"#");
		}
		break;
	}
	case '*':
	{
		if(m_wsChat.size() <= (size_t)15)
		{
			m_wsChat.append(L"*");
		}
		break;
	}
	case '+':
	{
		if(m_wsChat.size() <= (size_t)15)
		{
			m_wsChat.append(L"+");
		}
		break;
	}
	case '.':
	{
		if(m_wsChat.size() <= (size_t)20)
		{
			m_wsChat.append(L".");
		}
		break;
	}
	case ',':
	{
		if(m_wsChat.size() <= (size_t)20)
		{
			m_wsChat.append(L",");
		}
		break;
	}
	case '&':
	{
		if(m_wsChat.size() <= (size_t)14)
		{
			m_wsChat.append(L"&");
		}
		break;
	}
	case '~':
	{
		if(m_wsChat.size() <= (size_t)15)
		{
			m_wsChat.append(L"~");
		}
		break;
	}
	case '"':
	{
		if(m_wsChat.size() <= (size_t)15)
		{
			m_wsChat += L'"';
		}
		break;
	}
	case ';':
	{
		if(m_wsChat.size() <= (size_t)20)
		{
			m_wsChat.append(L";");
		}
		break;
	}
	case ':':
	{
		if(m_wsChat.size() <= (size_t)20)
		{
			m_wsChat.append(L":");
		}
		break;
	}
	default:
		break;
	}

}

void ChattingSystem::ProcessChatting(HWND hWnd, WPARAM wParam, LPARAM lParam, bool isInGame)
{
	if(wParam == VK_RETURN)
	{
		if (m_wsChat.size() > 0)
		{
#ifdef _WITH_SERVER_
			TCHAR* t;
			t = const_cast<TCHAR*>(m_wsChat.c_str());

			Network::GetInstance()->SendChattingText((char)Network::GetInstance()->GetMyID(), t);
#endif			
			m_wsChat.clear();
			m_wsChat.shrink_to_fit();
		}
	}
	if(wParam == VK_SPACE)
	{
		if(m_wsChat.size() <= (size_t)24)
		{
			m_wsChat.append(L" ");
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
		//�ѱ� �Է¿����� Ư������ ó���� �� �Լ����� ������Ѵ�.
		ProcessSpecialCharacter(wParam);

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

	m_dequeText.clear();
	m_wsChat.clear();
	m_wsChat.shrink_to_fit();

	if (m_pChattingFont)
		m_pChattingFont->Release();
	//for (int i = 0; i < m_maxChatSentenceCount; ++i)
	//{
	//	if (m_pd2dbrChatText[i]) m_pd2dbrChatText[i]->Release();
	//}
	//delete[] m_pd2dbrChatText;
	//m_pd2dbrChatText = nullptr;
	if (m_pd2dfxBitmapSource) m_pd2dfxBitmapSource->Release();
	if (m_pwicFormatConverter) m_pwicFormatConverter->Release();

}
#endif

void ChattingSystem::PushChattingText(const string& user,const char* chat)
{
	string s = user + ": ";
	s.append(chat);

	ZeroMemory(m_chat[m_nCurrentText], sizeof(256));
	int chattingLen = MultiByteToWideChar(CP_ACP, 0, s.c_str(), s.length(), NULL, NULL);
	MultiByteToWideChar(CP_ACP, 0, s.c_str(), s.length(), m_chat[m_nCurrentText], chattingLen);


	if(m_dequeText.size() >= m_maxChatSentenceCount-1)
	{
		//���� ���� ��ġ�� �ؽ�Ʈ�� pop�Ѵ�.
		m_dequeText.pop_back();
		m_dequeText.emplace_front(make_pair(m_chat[m_nCurrentText], (UINT32)chattingLen));
		m_nCurrentText = (++m_nCurrentText) % m_maxChatSentenceCount;
	}
	else
	{
		m_dequeText.emplace_front(make_pair(m_chat[m_nCurrentText], (UINT32)chattingLen));
		m_nCurrentText = (++m_nCurrentText) % m_maxChatSentenceCount;
	}

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

