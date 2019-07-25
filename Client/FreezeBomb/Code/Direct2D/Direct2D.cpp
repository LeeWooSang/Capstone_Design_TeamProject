#include "../Stdafx/Stdafx.h"
#include "Direct2D.h"
#include "../GameFramework/GameFramework.h"

void CDirect2D::CreateBitmapImage(ImageInfo info, string key)
{
	// [ Bitmap �̹��� �ʱ�ȭ ��� ] 
	// 1. Com��ü �ʱ�ȭ
	// 2. IWICImagingFactory ����
	// 3. Decoder ����
	// 4. �̹����� ������ ������
	// 5. Converter ����
	// 6. Bitmap ����

	IWICBitmapDecoder* pBitmapDecoder;
	HRESULT hResult = CGameFramework::GetWICImagingFactory()->CreateDecoderFromFilename(info.m_FilePath.c_str(), NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pBitmapDecoder);

	IWICBitmapFrameDecode* pFrameDecode;
	pBitmapDecoder->GetFrame(0, &pFrameDecode);

	IWICFormatConverter* pFormatConverter;
	CGameFramework::GetWICImagingFactory()->CreateFormatConverter(&pFormatConverter);
	pFormatConverter->Initialize(pFrameDecode, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0f, WICBitmapPaletteTypeCustom);

	hResult = CGameFramework::GetDeviceContext()->CreateBitmapFromWicBitmap(pFormatConverter, &info.m_Bitmap);
	m_ImageInfoMap.emplace(key, info);

	if (pBitmapDecoder)
		pBitmapDecoder->Release();

	if (pFrameDecode)
		pFrameDecode->Release();

	if (pFormatConverter)
		pFormatConverter->Release();
}

void CDirect2D::CreateGameFont()
{
	// ��Ʈ ���
	wstring fontPath[] = { L"../Resource/Font/a�ǿ��ǵ���.ttf", L"../Resource/Font/�����ý��丮.ttf" };

	// ��Ʈ�� ���� ��ġ�Ҷ� ���
	//AddFontResourceEx(fontPath[0].c_str(), FR_PRIVATE, 0);
	//AddFontResourceEx(fontPath[1].c_str(), FR_PRIVATE, 0);

	// ��Ʈ�� ��ġ���� �ʰ�, �޸𸮿� �÷��� ����� ���
	// ���� ����
	IDWriteFontSetBuilder1* pFontSetBuilder;
	HRESULT hResult = CGameFramework::GetWriteFactory()->CreateFontSetBuilder(&pFontSetBuilder);

	IDWriteFontFile* pFontFile[FONT::FontNum];
	IDWriteFontSet* pFontSet[FONT::FontNum];

	wstring FontName[FONT::FontNum];
	unsigned int max_length = 64;

	for (int i = 0; i < FONT::FontNum; ++i)
	{
		// �ش��ϴ� ��ο��� ��Ʈ ������ �ε��Ѵ�.
		hResult = CGameFramework::GetWriteFactory()->CreateFontFileReference(fontPath[i].c_str(), nullptr, &pFontFile[i]);
		// ������ ��Ʈ �߰�
		hResult = pFontSetBuilder->AddFontFile(pFontFile[i]);
		hResult = pFontSetBuilder->CreateFontSet(&pFontSet[i]);
		// ��Ʈ Collection�� ��Ʈ �߰� => ��Ʈ Collection���� ���� ����� ��Ʈ�� ����Ǿ� ����
		hResult = CGameFramework::GetWriteFactory()->CreateFontCollectionFromFontSet(pFontSet[i], &m_pFontCollection);

		// ��Ʈ �̸��� ������ ���
		IDWriteFontFamily* pFontFamily;
		IDWriteLocalizedStrings* pLocalizedFontName;

		hResult = m_pFontCollection->GetFontFamily(i, &pFontFamily);
		hResult = pFontFamily->GetFamilyNames(&pLocalizedFontName);
		// ����Ǿ��ִ� ��Ʈ�� �̸��� ����
		hResult = pLocalizedFontName->GetString(0, const_cast<wchar_t*>(FontName[i].c_str()), max_length);

		pFontFamily->Release();
		pLocalizedFontName->Release();
	}

	pFontSetBuilder->Release();

	for (int i = 0; i < FONT::FontNum; ++i)
	{
		pFontFile[i]->Release();
		pFontSet[i]->Release();
	}

	float dx = ((float)FRAME_BUFFER_WIDTH / (float)1280);
	float dy = ((float)FRAME_BUFFER_HEIGHT / (float)720);

	float dTotal = dx * dy;
	float fontSize = (25.f * dTotal);

	// ��Ʈ ��ü
	IDWriteTextFormat* pFont[FONT::FontNum];
	// ��Ʈ ����
	IDWriteTextLayout*	pTextLayout[FONT::FontNum];
	wstring wstr = L"TextLayout Initialize";

	for (int i = 0; i < FONT::FontNum; ++i)
	{
		// ��Ʈ ��ü ����	
		hResult = CGameFramework::GetWriteFactory()->CreateTextFormat(FontName[i].c_str(), m_pFontCollection, DWRITE_FONT_WEIGHT_DEMI_BOLD, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, fontSize, L"en-US", &pFont[i]);

		if (i == 0)
		{
			// ��Ʈ�� �߾ӿ� ���Ľ�Ű��
			hResult = pFont[i]->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
			hResult = pFont[i]->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
			hResult = CGameFramework::GetWriteFactory()->CreateTextLayout(wstr.c_str(), wstr.length(), pFont[i], 1024.0f, 1024.0f, &pTextLayout[i]);

			m_FontInfoMap.emplace("�ǿ��ǵ���", FontInfo(pFont[i], pTextLayout[i], fontSize));
		}
		else
		{
			hResult = pFont[i]->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
			hResult = pFont[i]->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
			hResult = CGameFramework::GetWriteFactory()->CreateTextLayout(wstr.c_str(), wstr.length(), pFont[i], 4096.0f, 4096.0f, &pTextLayout[i]);

			m_FontInfoMap.emplace("������", FontInfo(pFont[i], pTextLayout[i], fontSize));
		}
	}
}

void CDirect2D::CreateGameFontColor()
{
	ID2D1SolidColorBrush* pColor[FONT::FontColorNum];

	int index = 0;
	HRESULT hResult = CGameFramework::GetDeviceContext()->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::DeepPink, 1.0f), &pColor[index]);
	m_FontColorMap.emplace("��ȫ��", pColor[index++]);

	hResult = CGameFramework::GetDeviceContext()->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Brown, 1.0f), &pColor[index]);
	m_FontColorMap.emplace("����", pColor[index++]);

	hResult = CGameFramework::GetDeviceContext()->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White, 1.0f), &pColor[index]);
	m_FontColorMap.emplace("���", pColor[index++]);

	hResult = CGameFramework::GetDeviceContext()->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black, 1.0f), &pColor[index]);
	m_FontColorMap.emplace("������", pColor[index++]);

	hResult = CGameFramework::GetDeviceContext()->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::SkyBlue, 1.0f), &pColor[index]);
	m_FontColorMap.emplace("�ϴû�", pColor[index++]);

	hResult = CGameFramework::GetDeviceContext()->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::LavenderBlush, 1.0f), &pColor[index]);
	m_FontColorMap.emplace("����ȫ��", pColor[index++]);

	hResult = CGameFramework::GetDeviceContext()->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Red, 1.0f), &pColor[index]);
	m_FontColorMap.emplace("������", pColor[index++]);

	hResult = CGameFramework::GetDeviceContext()->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Orange, 1.0f), &pColor[index]);
	m_FontColorMap.emplace("��Ȳ��", pColor[index++]);
}

void CDirect2D::Release()
{
	//���� �ִ� ��Ʈ���̹����� ��� ����
	for (auto iter = m_ImageInfoMap.begin(); iter != m_ImageInfoMap.end(); )
	{
		(*iter).second.m_Bitmap->Release();
		iter = m_ImageInfoMap.erase(iter);
	}
	m_ImageInfoMap.clear();

	// ��Ʈ �÷��� ��� ����
	for (auto iter = m_FontColorMap.begin(); iter != m_FontColorMap.end(); )
	{
		(*iter).second->Release();
		iter = m_FontColorMap.erase(iter);
	}
	m_FontColorMap.clear();

	for (auto iter = m_FontInfoMap.begin(); iter != m_FontInfoMap.end(); )
	{
		(*iter).second.m_pFont->Release();
		(*iter).second.m_pTextLayout->Release();
		iter = m_FontInfoMap.erase(iter);
	}
	m_FontInfoMap.clear();

	if (m_pFontCollection)
		m_pFontCollection->Release();
}

void CDirect2D::Render()
{
	for (auto iter = m_ImageInfoMap.begin(); iter != m_ImageInfoMap.end(); )
		CGameFramework::GetDeviceContext()->DrawBitmap((*iter).second.m_Bitmap, (*iter).second.m_Pos);
}

void CDirect2D::Render(string key)
{
	auto iter = m_ImageInfoMap.find(key);
	if (iter != m_ImageInfoMap.end())
	{
		int width = (*iter).second.m_WidthPixel;
		int height = (*iter).second.m_HeightPixel;
		
		int totalX = (*iter).second.m_TotalFrameX;
		int totalY = (*iter).second.m_TotalFrameY;

		int frameX = (*iter).second.m_FrameXNum;
		int frameY = (*iter).second.m_FrameYNum;

		D2D1_RECT_F framePos;
		//< ��üũ�⸦ ��üX���������� ���� ������ �ѷ��� X�������� ũ�⸦ ���Ѵ�.
		int nX = width / totalX;	
		//< ��üũ�⸦ ��üY���������� ���� ������ �ѷ��� Y�������� ũ�⸦ ���Ѵ�.
		int nY = height / totalY;	

		//< ���������� �Ѹ� ��ġ�� ��´�. 
		framePos.left = nX * frameX;		
		framePos.top = nY * frameY;
		framePos.right = framePos.left + nX;
		framePos.bottom = framePos.top + nY;

		CGameFramework::GetDeviceContext()->DrawBitmap((*iter).second.m_Bitmap, (*iter).second.m_Pos, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, framePos);
	}
}

void CDirect2D::Render(string key, D2D1_RECT_F pos, int fx, int fy)
{
	auto iter = m_ImageInfoMap.find(key);
	if (iter != m_ImageInfoMap.end())
	{
		int width = (*iter).second.m_WidthPixel;
		int height = (*iter).second.m_HeightPixel;

		int totalX = (*iter).second.m_TotalFrameX;
		int totalY = (*iter).second.m_TotalFrameY;

		D2D1_RECT_F framePos;
		//< ��üũ�⸦ ��üX���������� ���� ������ �ѷ��� X�������� ũ�⸦ ���Ѵ�.
		int nX = width / totalX;
		//< ��üũ�⸦ ��üY���������� ���� ������ �ѷ��� Y�������� ũ�⸦ ���Ѵ�.
		int nY = height / totalY;

		//< ���������� �Ѹ� ��ġ�� ��´�. 
		framePos.left = nX * fx;
		framePos.top = nY * fy;
		framePos.right = framePos.left + nX;
		framePos.bottom = framePos.top + nY;

		CGameFramework::GetDeviceContext()->DrawBitmap((*iter).second.m_Bitmap, pos, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, framePos);
	}
}

void CDirect2D::Render(string font, string color, wstring text, D2D1_RECT_F pos)
{
	CGameFramework::GetDeviceContext()->DrawTextW(text.c_str(), text.length(), GetFontInfo(font).m_pFont, &pos, GetFontColor(color));
}

ImageInfo& CDirect2D::GetImageInfo(string key)
{
	auto iter = m_ImageInfoMap.find(key);
	if (iter != m_ImageInfoMap.end())
		return (*iter).second;
}

FontInfo&	 CDirect2D::GetFontInfo(string key)
{
	auto iter = m_FontInfoMap.find(key);
	if (iter != m_FontInfoMap.end())
		return (*iter).second;
}

ID2D1SolidColorBrush* CDirect2D::GetFontColor(string key)
{
	auto iter = m_FontColorMap.find(key);
	if (iter != m_FontColorMap.end())
		return (*iter).second;
}