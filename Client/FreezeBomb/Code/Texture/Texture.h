#pragma once

#define RESOURCE_TEXTURE2D				0x01
#define RESOURCE_TEXTURE2D_ARRAY	0x02		// [ ]
#define RESOURCE_TEXTURE2DARRAY	0x03
#define RESOURCE_TEXTURE_CUBE			0x04
#define RESOURCE_BUFFER						0x05

struct SRVROOTARGUMENTINFO
{
	int															m_nRootParameterIndex = 0;
	D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dSrvGpuDescriptorHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dUavGpuDescriptorHandle;
};

class CTexture
{
public:
	CTexture(int nTextureResources = 1, UINT nResourceType = RESOURCE_TEXTURE2D, int nSamplers = 0);
	virtual ~CTexture();

private:
	int								m_nReferences = 0;

	UINT							m_nTextureType = RESOURCE_TEXTURE2D;

	
	
	ID3D12Resource**	m_ppd3dTextureUploadBuffers;

	int															m_nSamplers = 0;
	D3D12_GPU_DESCRIPTOR_HANDLE*	m_pd3dSamplerGpuDescriptorHandles = NULL;

public:
	SRVROOTARGUMENTINFO*					m_pRootArgumentInfos = NULL;
	ID3D12Resource**						m_ppd3dTextures = NULL;
	int										m_nTextures = 0;
public:
	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }

	void SetRootArgumentUav(int nIndex, UINT nRootParaeterIndex, D3D12_GPU_DESCRIPTOR_HANDLE d3duavGpuDescriptorHandle);
	void SetRootArgument(int nIndex, UINT nRootParameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE d3dsrvGpuDescriptorHandle);
	void SetSampler(int nIndex, D3D12_GPU_DESCRIPTOR_HANDLE d3dSamplerGpuDescriptorHandle);

	void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);
	void UpdateShaderVariable(ID3D12GraphicsCommandList *pd3dCommandList, int nIndex);
	void ReleaseShaderVariables();

	ID3D12Resource *CreateTexture(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, UINT nWidth, UINT nHeight, DXGI_FORMAT dxgiFormat, D3D12_RESOURCE_FLAGS d3dResourceFlags, D3D12_RESOURCE_STATES d3dResourceStates, D3D12_CLEAR_VALUE *pd3dClearValue, UINT nIndex);
	
	void LoadTextureFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, wchar_t *pszFileName, UINT nIndex, bool bIsDDSFile = true);

	int GetTextures() { return(m_nTextures); }
	ID3D12Resource *GetTexture(int nIndex) { return(m_ppd3dTextures[nIndex]); }
	UINT GetTextureType() { return(m_nTextureType); }

	void ReleaseUploadBuffers();
};
