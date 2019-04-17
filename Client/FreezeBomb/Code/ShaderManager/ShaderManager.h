#pragma once

class CShader;
class CPlayer;
class CCamera;
class CResourceManager;
class CTextureManager;
class CSobelCartoonShader;
class CShaderManager
{
public:
	CShaderManager();
	~CShaderManager();


	void Initialize(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, const int& nPlayerCount);
	void AnimateObjects(float fTimeElapsed, CCamera* pCamera, CPlayer* pPlayer = nullptr);
	void ReleaseObjects();
	void ReleaseUploadBuffers();
	
	void PostRender(ID3D12GraphicsCommandList* pd3dCommandList, float fTimeElapsed, CCamera* pCamera);
	void PreRender(ID3D12GraphicsCommandList *pd3dCommandList,float fTimeElapsed, CCamera *pCamera = nullptr);


	void ProcessCollision(XMFLOAT3& position);				//�浹 ���� ó�� �Լ�

	const map<string, CShader*>& getShaderMap()	const { return m_ShaderMap; }
	CResourceManager* getResourceManager()	const { return m_pResourceManager; }

private:

	size_t							m_nShaders = 0;
	//snow�� ui���� postó�� �ȵǾ��ϱ� ������ m_nShader - m_nPostShader ���� postshader������ �ľǵǾ�� ��
	size_t							m_nPostShaders = 0;
	CShader**					m_ppShaders{ nullptr };

	map<string, CShader*>	m_ShaderMap;

	CResourceManager*			m_pResourceManager{ nullptr };

	CPlayer*	m_pPlayer{ nullptr };
};