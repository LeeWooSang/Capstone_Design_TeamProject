#include "../Stdafx/Stdafx.h"
#include "Direct2D.h"
#include "../GameFramework/GameFramework.h"

void CDirect2D::CreateBitmapImage(ImageInfo info,const string& key)
{
	// [ Bitmap 이미지 초기화 방법 ] 
	// 1. Com객체 초기화
	// 2. IWICImagingFactory 생성
	// 3. Decoder 생성
	// 4. 이미지의 프레임 얻어오기
	// 5. Converter 생성
	// 6. Bitmap 생성

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
	// 폰트 경로
	wstring fontPath[] = { L"../Resource/Font/a피오피동글.ttf", L"../Resource/Font/메이플스토리.ttf" };

	// 폰트를 직접 설치할때 사용
	//AddFontResourceEx(fontPath[0].c_str(), FR_PRIVATE, 0);
	//AddFontResourceEx(fontPath[1].c_str(), FR_PRIVATE, 0);

	// 폰트를 설치하지 않고, 메모리에 올려서 사용할 경우
	// 빌더 생성
	IDWriteFontSetBuilder1* pFontSetBuilder;
	HRESULT hResult = CGameFramework::GetWriteFactory()->CreateFontSetBuilder(&pFontSetBuilder);

	IDWriteFontFile* pFontFile[FONT::FontNum];
	IDWriteFontSet* pFontSet[FONT::FontNum];

	wstring FontName[FONT::FontNum];
	unsigned int max_length = 64;

	for (int i = 0; i < FONT::FontNum; ++i)
	{
		// 해당하는 경로에서 폰트 파일을 로드한다.
		hResult = CGameFramework::GetWriteFactory()->CreateFontFileReference(fontPath[i].c_str(), nullptr, &pFontFile[i]);
		// 빌더에 폰트 추가
		hResult = pFontSetBuilder->AddFontFile(pFontFile[i]);
		hResult = pFontSetBuilder->CreateFontSet(&pFontSet[i]);
		// 폰트 Collection에 폰트 추가 => 폰트 Collection에서 내가 사용할 폰트가 저장되어 있음
		hResult = CGameFramework::GetWriteFactory()->CreateFontCollectionFromFontSet(pFontSet[i], &m_pFontCollection);

		// 폰트 이름을 얻어오는 방법
		IDWriteFontFamily* pFontFamily;
		IDWriteLocalizedStrings* pLocalizedFontName;

		hResult = m_pFontCollection->GetFontFamily(i, &pFontFamily);
		hResult = pFontFamily->GetFamilyNames(&pLocalizedFontName);
		// 저장되어있는 폰트의 이름을 얻어옴
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

	// 폰트 객체
	IDWriteTextFormat* pFont[FONT::FontNum];
	// 폰트 형식
	IDWriteTextLayout*	pTextLayout[FONT::FontNum];
	wstring wstr = L"TextLayout Initialize";

	for (int i = 0; i < FONT::FontNum; ++i)
	{
		// 폰트 객체 생성	
		hResult = CGameFramework::GetWriteFactory()->CreateTextFormat(FontName[i].c_str(), m_pFontCollection, DWRITE_FONT_WEIGHT_DEMI_BOLD, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, fontSize, L"en-US", &pFont[i]);

		if (i == 0)
		{
			// 폰트를 중앙에 정렬시키기
			hResult = pFont[i]->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
			hResult = pFont[i]->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
			hResult = CGameFramework::GetWriteFactory()->CreateTextLayout(wstr.c_str(), wstr.length(), pFont[i], 1024.0f, 1024.0f, &pTextLayout[i]);

			m_FontInfoMap.emplace("피오피동글", FontInfo(pFont[i], pTextLayout[i], fontSize));
		}
		else
		{
			hResult = pFont[i]->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
			hResult = pFont[i]->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
			hResult = CGameFramework::GetWriteFactory()->CreateTextLayout(wstr.c_str(), wstr.length(), pFont[i], 4096.0f, 4096.0f, &pTextLayout[i]);

			m_FontInfoMap.emplace("메이플", FontInfo(pFont[i], pTextLayout[i], fontSize));
		}
	}
}

void CDirect2D::CreateGameFontColor()
{
	ID2D1SolidColorBrush* pColor[FONT::FontColorNum];

	int index = 0;
	HRESULT hResult = CGameFramework::GetDeviceContext()->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::DeepPink, 1.0f), &pColor[index]);
	m_FontColorMap.emplace("분홍색", pColor[index++]);

	hResult = CGameFramework::GetDeviceContext()->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Brown, 1.0f), &pColor[index]);
	m_FontColorMap.emplace("갈색", pColor[index++]);

	hResult = CGameFramework::GetDeviceContext()->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White, 1.0f), &pColor[index]);
	m_FontColorMap.emplace("흰색", pColor[index++]);

	hResult = CGameFramework::GetDeviceContext()->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black, 1.0f), &pColor[index]);
	m_FontColorMap.emplace("검은색", pColor[index++]);

	hResult = CGameFramework::GetDeviceContext()->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::SkyBlue, 1.0f), &pColor[index]);
	m_FontColorMap.emplace("하늘색", pColor[index++]);

	hResult = CGameFramework::GetDeviceContext()->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::LavenderBlush, 1.0f), &pColor[index]);
	m_FontColorMap.emplace("연분홍색", pColor[index++]);

	hResult = CGameFramework::GetDeviceContext()->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Red, 1.0f), &pColor[index]);
	m_FontColorMap.emplace("빨간색", pColor[index++]);

	hResult = CGameFramework::GetDeviceContext()->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Lime, 1.0f), &pColor[index]);
	m_FontColorMap.emplace("라임색", pColor[index++]);

	hResult = CGameFramework::GetDeviceContext()->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Gold, 1.0f), &pColor[index]);
	m_FontColorMap.emplace("황금색", pColor[index++]);

	hResult = CGameFramework::GetDeviceContext()->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Navy, 1.0f), &pColor[index]);
	m_FontColorMap.emplace("네이비", pColor[index++]);

}

void CDirect2D::Release()
{
	//갖고 있는 비트맵이미지를 모두 지움
	for (auto iter = m_ImageInfoMap.begin(); iter != m_ImageInfoMap.end(); )
	{
		(*iter).second.m_Bitmap->Release();
		iter = m_ImageInfoMap.erase(iter);
	}
	m_ImageInfoMap.clear();

	// 폰트 컬러를 모두 지움
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

void CDirect2D::Render(const string& key)
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
		//< 전체크기를 전체X프레임으로 나눠 앞으로 뿌려줄 X프레임의 크기를 구한다.
		int nX = width / totalX;	
		//< 전체크기를 전체Y프레임으로 나눠 앞으로 뿌려줄 Y프레임의 크기를 구한다.
		int nY = height / totalY;	

		//< 최종적으로 뿌릴 위치를 담는다. 
		framePos.left = nX * frameX;		
		framePos.top = nY * frameY;
		framePos.right = framePos.left + nX;
		framePos.bottom = framePos.top + nY;

		CGameFramework::GetDeviceContext()->DrawBitmap((*iter).second.m_Bitmap, (*iter).second.m_Pos, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, framePos);
	}
}

void CDirect2D::Render(const string& key, D2D1_RECT_F pos, int fx, int fy)
{
	auto iter = m_ImageInfoMap.find(key);
	if (iter != m_ImageInfoMap.end())
	{
		int width = (*iter).second.m_WidthPixel;
		int height = (*iter).second.m_HeightPixel;

		int totalX = (*iter).second.m_TotalFrameX;
		int totalY = (*iter).second.m_TotalFrameY;

		D2D1_RECT_F framePos;
		//< 전체크기를 전체X프레임으로 나눠 앞으로 뿌려줄 X프레임의 크기를 구한다.
		int nX = width / totalX;
		//< 전체크기를 전체Y프레임으로 나눠 앞으로 뿌려줄 Y프레임의 크기를 구한다.
		int nY = height / totalY;

		//< 최종적으로 뿌릴 위치를 담는다. 
		framePos.left = nX * fx;
		framePos.top = nY * fy;
		framePos.right = framePos.left + nX;
		framePos.bottom = framePos.top + nY;

		CGameFramework::GetDeviceContext()->DrawBitmap((*iter).second.m_Bitmap, pos, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, framePos);
	}
}

void CDirect2D::Render(const string& font, const string& color, const wstring& text, D2D1_RECT_F pos)
{
	CGameFramework::GetDeviceContext()->DrawTextW(text.c_str(), text.length(), GetFontInfo(font).m_pFont, &pos, GetFontColor(color));
}

ImageInfo& CDirect2D::GetImageInfo(const string& key)
{
	auto iter = m_ImageInfoMap.find(key);
	if (iter != m_ImageInfoMap.end())
		return (*iter).second;
}

FontInfo&	 CDirect2D::GetFontInfo(const string& key)
{
	auto iter = m_FontInfoMap.find(key);
	if (iter != m_FontInfoMap.end())
		return (*iter).second;
}

ID2D1SolidColorBrush* CDirect2D::GetFontColor(const string& key)
{
	auto iter = m_FontColorMap.find(key);
	if (iter != m_FontColorMap.end())
		return (*iter).second;
}