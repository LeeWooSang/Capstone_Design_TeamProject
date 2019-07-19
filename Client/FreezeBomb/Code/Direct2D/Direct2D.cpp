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

void CDirect2D::Release()
{
	//���� �ִ� ��Ʈ���̹����� ��� ����
	for (auto iter = m_ImageInfoMap.begin(); iter != m_ImageInfoMap.end(); )
	{
		(*iter).second.m_Bitmap->Release();
		iter = m_ImageInfoMap.erase(iter);
	}
	m_ImageInfoMap.clear();
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

ImageInfo& CDirect2D::GetImageInfo(string key)
{
	auto iter = m_ImageInfoMap.find(key);
	if (iter != m_ImageInfoMap.end())
		return (*iter).second;
}

