#include "../../Stdafx/Stdafx.h"
#include "Surrounding.h"
#include "../Shadow/Shadow.h"
#include "../../Material/Material.h"
#include "../../Shader/Shader.h"
#include "../../FrameTransform/FrameTransform.h"

//ID3D12RootSignature* pd3dGraphicsRootSignature �ʿ���� ���� - ���ڷ� �ȳѰ��ִ°� ���� ����
CSurrounding::CSurrounding(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature)
{
}

CSurrounding::~CSurrounding()
{
	if (m_pShadow)
		delete m_pShadow;
}

void CSurrounding::Initialize_Shadow(CLoadedModelInfo* pLoadedModel, CGameObject* pGameObject)
{
	m_pShadow = new CShadow(pLoadedModel, this);
}

void CSurrounding::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera, int nPipelineState)
{
	if (IsVisible(pCamera) == true)
	{
		CGameObject::Render(pd3dCommandList, pCamera, GameObject);

		if (m_pShadow)
			m_pShadow->Render(pd3dCommandList, pCamera, GameObject_Shadow);
	}
}