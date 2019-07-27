#pragma once

struct Bounds
{
	string m_Name;
	XMFLOAT3 m_xmf3Center = XMFLOAT3(0.f, 0.f, 0.f);
	XMFLOAT3 m_xmf3Extent = XMFLOAT3(0.f, 0.f, 0.f);
};

struct MapObjectInfo
{
	string m_Name;
	XMFLOAT3 m_Position = XMFLOAT3(0.f, 0.f, 0.f);
	XMFLOAT3 m_Look = XMFLOAT3(0.f, 0.f, 0.f);
	XMFLOAT3 m_Up = XMFLOAT3(0.f, 0.f, 0.f);
	XMFLOAT3 m_Right = XMFLOAT3(0.f, 0.f, 0.f);
};

struct TextureInfo
{
	TextureInfo(unsigned int resourceType, wstring filename, int rootparameter) 
		: m_ResourceType(resourceType), m_Filename(filename), m_RootParameter(rootparameter) {}

	unsigned int m_ResourceType;
	wstring			m_Filename;
	int					m_RootParameter;
	size_t			m_FileSize = 0;
};

struct ModelInfo
{
	ModelInfo(string filename, bool hasAnimation) : m_Filename(filename), m_HasAnimation(hasAnimation) { }

	string m_Filename;
	bool m_HasAnimation;
};

using RoundInfo = multimap<string, MapObjectInfo*>;
class CTexture;
class CLoadedModelInfo;
// ���� �Ǵ� ���ҽ����� �����ϱ� ���� Ŭ����
class CResourceManager
{
public:
	CResourceManager();
	~CResourceManager();

	void Initialize(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature);

	static void PrepareSound();
	void PrepareLoad();
	void LoadTexture(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	void LoadModel(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);
	void LoadMapObjectInfo(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	void LoadCharactersInfo(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);

	void LoadBound(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	void ReleaseModel();
	void Release();
	void ReleaseUploadBuffers();

	const map<string, CTexture*>& getTextureMap()	const { return m_TextureMap; }
	const map<string, CLoadedModelInfo*>& getModelMap()	const { return m_ModelMap; }
	const map<string, Bounds*>& getBoundMap()	const { return m_BoundMap; }
	// ���忡 �ش��ϴ� �� ��ġ ���� ��ȯ
	const multimap<string, MapObjectInfo*>& getMapObjectInfo()	const {}

	const unordered_map<unsigned char, RoundInfo>& getRoundMapObjectInfo()	const { return m_RoundInfoMapObjectInfo; }

	static const XMFLOAT3& GetCharactersPosition(int index) { return m_CharactersInfoMap[index]->m_Position; }

private:
	map<string, CTexture*>					m_TextureMap;
	map<string, CLoadedModelInfo*>	m_ModelMap;
	map<string, Bounds*>						m_BoundMap;

	multimap<string, MapObjectInfo*>	m_MapObjectInfoMultiMap;

	// ���ҽ� �̸��� ����
	map<string, TextureInfo>		m_TextureInfoMap;
	map<string, ModelInfo>		m_ModelInfoMap;

	unordered_map<unsigned char, RoundInfo> m_RoundInfoMapObjectInfo;

	static vector<MapObjectInfo*> m_CharactersInfoMap;
};