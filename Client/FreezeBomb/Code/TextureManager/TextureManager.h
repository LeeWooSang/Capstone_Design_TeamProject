#pragma once

// ���� �Ǵ� �ؽ�ó���� �����ϱ� ���� Ŭ����
class CTexture;
class CTextureManager
{
public:
	CTextureManager();
	~CTextureManager();

	void Initialize(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	void Release();

	const map<string, CTexture*>& getTextureMap()	const { return m_TextureMap; }

private:
	map<string, CTexture*> m_TextureMap;
};