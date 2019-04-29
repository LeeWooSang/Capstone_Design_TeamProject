#include "../Stdafx/Stdafx.h"
#include <string>
#include "Chatting.h"

#ifdef _WITH_DIRECT2D_

ChattingSystem::ChattingSystem()
{
}

ChattingSystem::~ChattingSystem()
{
	cout << "Chattinng System �Ҹ�" << endl;

}
void ChattingSystem::Initialize(IDWriteFactory* writeFactory,ID2D1DeviceContext2* pd2dDeviceContext)
{
	HRESULT hResult;
	m_pdwChattingFont = new IDWriteTextFormat*[m_maxChatSentenceCount];
	m_pd2dbrChatText = new ID2D1SolidColorBrush*[m_maxChatSentenceCount];

	for (int i = 0; i < m_maxChatSentenceCount; ++i)
	{
		hResult = writeFactory->CreateTextFormat(L"���", nullptr, DWRITE_FONT_WEIGHT_DEMI_BOLD, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 30.0f, L"en-US", &m_pdwChattingFont[i]);
		hResult = m_pdwChattingFont[i]->SetTextAlignment( DWRITE_TEXT_ALIGNMENT_LEADING);
		hResult = m_pdwChattingFont[i]->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
		hResult = writeFactory->CreateTextLayout(L"�ؽ�Ʈ ���̾ƿ�", 64, m_pdwChattingFont[i], 4096.0f, 4096.0f, &m_pdwChattingLayout);
		pd2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black, 1.0f), &m_pd2dbrChatText[i]);
	
	}
}

void ChattingSystem::ProcessChatting(UCHAR* key)
{
	TCHAR		localChat[512];
	static bool bCapital = false;	// true : �빮�� , false  = �ҹ���
	
	//n_tcscpy_s()
	//���� ������ Ȱ��ȭ ���� ���
	if (GetIMEMode() == IME_CMODE_ALPHANUMERIC)
	{
		if (GetKeyState(VK_CAPITAL) & 0x8001 && bCapital == true)
		{
			if (GetKeyState(VK_C) & 0x8000)
			{
				m_sChat += 'C';
			}
		}
		else
		{
			if(GetKeyState(VK_C) & 0x8000)
			{
				m_sChat += 'c';
			}
		}
		

	}
	else if(GetIMEMode() == IME_CMODE_NATIVE)
	{
		//���� �ѱ��� Ȱ��ȭ ���� ���
		if (key[VK_C] & 0xF0)
		{

		}
	}

	
}

void ChattingSystem::Destroy()
{
	for (UINT i = 0; i < m_maxChatSentenceCount; ++i)
	{
		if (m_pdwChattingFont[i]) m_pdwChattingFont[i]->Release();
		if (m_pd2dbrChatText[i]) m_pd2dbrChatText[i]->Release();
	}
	delete[]m_pdwChattingFont;
	m_pdwChattingFont = nullptr;
	delete[]m_pd2dbrChatText;
	m_pd2dbrChatText = nullptr;
}
#endif

//���α׷� ������ �ѿ� ��ȯ
void ChattingSystem::SetIMEMode(HWND hWnd,bool bHanMode)
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

