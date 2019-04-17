#pragma once

#include "../BillboardShader.h"
#include "../../../Cube/Cube.h"

class CTexture;
class CMaterial;
class CPlayer;
class CTerrain;
class CSnowShader : public CBillboardShader
{
public:
	CSnowShader();
	virtual ~CSnowShader();

	virtual D3D12_SHADER_BYTECODE CreateVertexShader();
	virtual D3D12_SHADER_BYTECODE CreatePixelShader();

	virtual void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, 
		const map<string, CTexture*>& Context, void* pContext);
	virtual void AnimateObjects(float fTimeElapsed, CCamera *pCamera, CPlayer* pPlayer = NULL);
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera, int nPipelineState);

	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseShaderVariables();

private:
	float					m_fTimeLagScale = 0.05f;

	CMaterial*		m_pMaterial{ nullptr };

	CCube				m_refCubeObject;
	CPlayer*			m_pPlayer{ nullptr };		//�÷��̾� ��ġ ��ó���� ��� �������� �ϱ� ������ 

	CTerrain*			m_pTerrain{ nullptr };	//�� ������Ʈ�� �ͷ��ΰ� �浹�ϸ� �ٽ� ���������� ��������.

	struct InstancingData
	{
		XMFLOAT4X4 m_World;
	};

	ID3D12Resource*				m_pd3dInstancingData{ nullptr };
	InstancingData*				m_pMappedInstancingData{ nullptr };
};