#include "../../Stdafx/Stdafx.h"
#include "Shadow.h"

CShadow::CShadow(CLoadedModelInfo* pLoadedModel, CGameObject* pGameObject)
{
	if(pLoadedModel != nullptr)
		SetChild(pLoadedModel->m_pModelRootObject, true);

	if (pGameObject != nullptr)
	{
		m_pTruth = pGameObject;

		if (m_pTruth->m_pAnimationController)
			m_pAnimationController = m_pTruth->m_pAnimationController;
		if (m_pTruth->m_pSkinningBoneTransforms)
			m_pSkinningBoneTransforms = m_pTruth->m_pSkinningBoneTransforms;
		if (m_pTruth->m_pFrameTransform)
			m_pFrameTransform = m_pTruth->m_pFrameTransform;
		if (m_pTruth->m_pMesh)
			m_pMesh = m_pTruth->m_pMesh;
	}
}

CShadow::~CShadow()
{
	if (m_pAnimationController)
	{
		m_pTruth->m_pAnimationController = nullptr;
		delete m_pAnimationController;
		m_pAnimationController = nullptr;
	}

	if (m_pSkinningBoneTransforms)
	{
		m_pTruth->m_pSkinningBoneTransforms = nullptr;
		delete m_pSkinningBoneTransforms;
		m_pSkinningBoneTransforms = nullptr;
	}

	if (m_pFrameTransform)
	{
		m_pTruth->m_pFrameTransform = nullptr;
		delete m_pFrameTransform;
		m_pFrameTransform = nullptr;
	}

	if (m_pMesh)
	{
		m_pTruth->m_pMesh = nullptr;
		m_pMesh->Release();
		m_pMesh = nullptr;
	}
}

// 정적인 오브젝트 그림자
void CShadow::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState)
{
	CGameObject::Render(pd3dCommandList, pCamera, nPipelineState);
}

// 큐브 파티클 그림자
void CShadow::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState, int nInstance)
{
	CGameObject::Render(pd3dCommandList, pCamera, nPipelineState, nInstance);
}

void CShadow::Item_Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int ItemType, int nPipelineState)
{
	CGameObject::Item_Render(pd3dCommandList, pCamera, ItemType, nPipelineState);
}

void CShadow::Tagger_Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera* pCamera, int matID, bool HasGoldTimer, int nPipelineState)
{
	CGameObject::Tagger_Render(pd3dCommandList, pCamera, matID, HasGoldTimer, nPipelineState);
}

// 도망자의 그림자
void CShadow::RunAway_Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera* pCamera, int matID, bool isICE, bool HasHammer, bool HasGoldHammer, int nPipelineState)
{
	CGameObject::RunAway_Render(pd3dCommandList, pCamera, matID, isICE, HasHammer, HasGoldHammer,false ,nPipelineState);
}
