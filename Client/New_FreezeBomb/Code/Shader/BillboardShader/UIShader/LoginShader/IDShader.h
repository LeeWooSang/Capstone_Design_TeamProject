#pragma once
#include "../UIShader.h"

#ifdef _WITH_SERVER_

class CTexture;
class CMaterial;
class CUI;


class CIDShader : public CUIShader
{
public:

	CIDShader();
	virtual ~CIDShader();

	virtual void CreateShader(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature);
	virtual D3D12_SHADER_BYTECODE CreateVertexShader();
	virtual D3D12_SHADER_BYTECODE CreatePixelShader();

	void CreateCbvSrvDescriptorHeaps(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, int nConstantBufferViews, int nShaderResourceViews);
	void CreateShaderResourceViews(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CTexture *pTexture, UINT nRootParameterStartIndex, bool bAutoIncrement);
	void CreateConstantBufferViews(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, int nConstantBufferViews, ID3D12Resource *pd3dConstantBuffers, UINT nStride);

	virtual void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, void *pContext);
	virtual void AnimateObjects(float fTimeElapsed);
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList,int nPipelineStates,bool isInput);
	virtual void ReleaseObjects();
	virtual void ReleaseUploadBuffers();

		//cpu,gpu ��ũ���� �ڵ��� ��ȯ���ִ� �Լ��� ���� �ʿ� 
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandleForHeapStart() { return(m_pd3dCbvSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart()); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandleForHeapStart() { return(m_pd3dCbvSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart()); }

	//��� ���ۿ� ��ũ���� �ڵ�� ���̴� ���ҽ��� ��ũ���� �ڵ� �ʿ�
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUCbvDescriptorStartHandle() { return(m_d3dCbvCPUDescriptorStartHandle); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUCbvDescriptorStartHandle() { return(m_d3dCbvGPUDescriptorStartHandle); }
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUSrvDescriptorStartHandle() { return(m_d3dSrvCPUDescriptorStartHandle); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUSrvDescriptorStartHandle() { return(m_d3dSrvGPUDescriptorStartHandle); }

	
	enum state{NO_SELECT,ID_SELECT,REQUEST_LOGIN};
	
	int DecideTextureByCursor(const RECT& rect,const LONG& mouseX,const LONG& mouseY);
	//����ڰ� � �����̸� Ŭ���ߴ��� 
	
protected:
	int			m_currentTexture{ NO_SELECT };
	
	ID3D12DescriptorHeap					*m_pd3dCbvSrvDescriptorHeap = NULL;			//cbv,srv�� ������ ��

	D3D12_CPU_DESCRIPTOR_HANDLE				m_d3dCbvCPUDescriptorStartHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE				m_d3dCbvGPUDescriptorStartHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE				m_d3dSrvCPUDescriptorStartHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE				m_d3dSrvGPUDescriptorStartHandle;
	std::vector<CTexture*> vTexture;

	CTexture** pLoginTextures{ nullptr };
};
#endif