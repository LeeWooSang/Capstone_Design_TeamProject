#pragma once

#include "../UIShader.h"

//lobby
class CTexture;
class CMaterial;
class CUI;
class CSoundSystem;
class CCharacterSelectUIShader: public CUIShader
{
public:
	CCharacterSelectUIShader();
	virtual ~CCharacterSelectUIShader();

	virtual void CreateShader(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);
	virtual D3D12_SHADER_BYTECODE CreateVertexShader();
	virtual D3D12_SHADER_BYTECODE CreatePixelShader();
	
	void CreateCbvSrvDescriptorHeaps(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, int nConstantBufferViews, int nShaderResourceViews);
	void CreateShaderResourceViews(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CTexture *pTexture, UINT nRootParameterStartIndex, bool bAutoIncrement);
	void CreateConstantBufferViews(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, int nConstantBufferViews, ID3D12Resource *pd3dConstantBuffers, UINT nStride);

	virtual void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, void *pContext);
//	virtual void AnimateObjects(float fTimeElapsed);
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList,int nPipelineStates);
	virtual void ReleaseObjects();

	void DecideTextureByCursorPosition(CSoundSystem* sound,UINT nMessageID,float mouseX, float mouseY);
	
	byte SelectedCharacter() const { return m_characterSelect; };

		//cpu,gpu ��ũ���� �ڵ��� ��ȯ���ִ� �Լ��� ���� �ʿ� 
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandleForHeapStart() { return(m_pd3dCbvSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart()); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandleForHeapStart() { return(m_pd3dCbvSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart()); }

	//��� ���ۿ� ��ũ���� �ڵ�� ���̴� ���ҽ��� ��ũ���� �ڵ� �ʿ�
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUCbvDescriptorStartHandle() { return(m_d3dCbvCPUDescriptorStartHandle); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUCbvDescriptorStartHandle() { return(m_d3dCbvGPUDescriptorStartHandle); }
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUSrvDescriptorStartHandle() { return(m_d3dSrvCPUDescriptorStartHandle); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUSrvDescriptorStartHandle() { return(m_d3dSrvGPUDescriptorStartHandle); }

	enum LOBBY_CHARACTERSEL { PINK, BROWN, WHITE, BLACK, BLUE, PANDA, BASE, READY, QUIT };
	
	bool IsReady() { return isReady; }

#ifdef _WITH_SERVER_
	bool isSelectDone() { return isCharacterSelectDone; }
	bool  isCharacterSelectDone{ false };
#endif
private:
	CTexture** pSelectTextures;

	ID3D12DescriptorHeap*						m_pd3dCbvSrvDescriptorHeap = nullptr;			//cbv,srv�� ������ ��

	D3D12_CPU_DESCRIPTOR_HANDLE		m_d3dCbvCPUDescriptorStartHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dCbvGPUDescriptorStartHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE		m_d3dSrvCPUDescriptorStartHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dSrvGPUDescriptorStartHandle;
	
	vector<CTexture*> vTexture;

	bool							isReady{ false };
	int								m_currentTexture;
	byte							m_characterSelect;


};