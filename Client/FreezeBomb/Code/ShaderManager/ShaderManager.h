#pragma once

class CShader;
class CPlayer;
class CCamera;
class CTextureManager;
class CShaderManager
{
public:
	CShaderManager();
	~CShaderManager();

	void Initialize(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);
	void AnimateObjects(float fTimeElapsed, CCamera* pCamera, CPlayer* pPlayer = nullptr);
	void ReleaseObjects();
	void ReleaseUploadBuffers();
	void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera = nullptr);

	const map<string, CShader*>& getShaderMap()	const { return m_ShaderMap; }

private:
	size_t						m_nShaders = 0;
	CShader**				m_ppShaders{ nullptr };

	map<string, CShader*>	m_ShaderMap;

	CTextureManager*			m_pTextureManager{ nullptr };
};