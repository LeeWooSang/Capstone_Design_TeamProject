#include "../../../../Stdafx/Stdafx.h"
#include "SkinnedAnimationObjectShader.h"
#include "../../../../ResourceManager/ResourceManager.h"
#include "../../../../GameObject/GameObject.h"
#include "../../../../GameObject/Terrain/Terrain.h"
#include "../../../../GameObject/EvilBear/EvilBear.h"
#include "../../../../GameObject/Shadow/Shadow.h"

CSkinnedAnimationObjectShader::CSkinnedAnimationObjectShader()
{
}

CSkinnedAnimationObjectShader::~CSkinnedAnimationObjectShader()
{
}

void CSkinnedAnimationObjectShader::BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature,
	const map<string, Bounds*>& Context,const int& nPlayerCount, void* pContext)
{
	CLoadedModelInfo* pEvilBearModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature,
		"../Resource/Models/EvilBear.bin", nullptr, true, "Enemy");

	CTerrain* pTerrain = (CTerrain *)pContext;

	m_nObjects = nPlayerCount;
	m_ppObjects = new CGameObject*[m_nObjects];

	m_ppObjects[0] = new CEvilBear(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, CGameObject::MATERIALTYPE::PINK);
	m_ppObjects[0]->SetChild(pEvilBearModel->m_pModelRootObject, true);
	m_ppObjects[0]->m_pAnimationController = new CAnimationController(2, pEvilBearModel->m_pAnimationSets);
	// 0�� Ʈ���� 0�� �ִϸ��̼��� Set
	m_ppObjects[0]->m_pAnimationController->SetTrackAnimationSet(0, 0);
	// 1�� Ʈ���� 1�� �ִϸ��̼��� Set
	m_ppObjects[0]->m_pAnimationController->SetTrackAnimationSet(1, 1);
	// 0�� Ʈ���� ����ġ�� 80%
	m_ppObjects[0]->m_pAnimationController->SetTrackWeight(0, 0.8f);
	// 1�� Ʈ���� ����ġ�� 20% 
	m_ppObjects[0]->m_pAnimationController->SetTrackWeight(1, 0.2f);
	m_ppObjects[0]->m_pSkinningBoneTransforms = new CSkinningBoneTransforms(pd3dDevice, pd3dCommandList, pEvilBearModel);


	m_ppObjects[1] = new CEvilBear(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, CGameObject::MATERIALTYPE::BLACK);
	m_ppObjects[1]->SetChild(pEvilBearModel->m_pModelRootObject, true);
	m_ppObjects[1]->m_pAnimationController = new CAnimationController(1, pEvilBearModel->m_pAnimationSets);
	//m_ppObjects[1]->m_pAnimationController->SetTrackAnimationSet(1, 2);
	m_ppObjects[1]->m_pAnimationController->SetTrackAnimationSet(0, 9);
	m_ppObjects[1]->m_pAnimationController->SetTrackSpeed(0, 0.8f);
	m_ppObjects[1]->m_pAnimationController->SetTrackWeight(0, 0.9);
	//m_ppObjects[1]->m_pAnimationController->SetTrackWeight(1, 0.1);
	// �ִϸ��̼��� �ӵ��� 0.25�� �־ ������ �ִϸ��̼� ������ �ϵ��� Set
	m_ppObjects[1]->m_pAnimationController->SetTrackSpeed(0, 0.25f);
	m_ppObjects[1]->m_pSkinningBoneTransforms = new CSkinningBoneTransforms(pd3dDevice, pd3dCommandList, pEvilBearModel);




	m_ppObjects[2] = new CEvilBear(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, CGameObject::MATERIALTYPE::BROWN);
	m_ppObjects[2]->SetChild(pEvilBearModel->m_pModelRootObject, true);
	m_ppObjects[2]->m_pAnimationController = new CAnimationController(1, pEvilBearModel->m_pAnimationSets);
	m_ppObjects[2]->m_pAnimationController->SetTrackAnimationSet(0, 0);
	// �ִϸ��̼��� ������ġ�� �ٸ��� �ش�. �׷��� ���� �����̴��� �ٸ��� �ִϸ��̼���
	m_ppObjects[2]->m_pAnimationController->SetTrackPosition(0, 0.95f);
	m_ppObjects[2]->m_pSkinningBoneTransforms = new CSkinningBoneTransforms(pd3dDevice, pd3dCommandList, pEvilBearModel);



	m_ppObjects[3] = new CEvilBear(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, CGameObject::MATERIALTYPE::BLUE);
	m_ppObjects[3]->SetChild(pEvilBearModel->m_pModelRootObject, true);
	m_ppObjects[3]->m_pAnimationController = new CAnimationController(1, pEvilBearModel->m_pAnimationSets);
	m_ppObjects[3]->m_pAnimationController->SetTrackAnimationSet(0, 0);
	m_ppObjects[3]->m_pSkinningBoneTransforms = new CSkinningBoneTransforms(pd3dDevice, pd3dCommandList, pEvilBearModel);



	m_ppObjects[4] = new CEvilBear(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, CGameObject::MATERIALTYPE::ICEMAT);
	m_ppObjects[4]->SetChild(pEvilBearModel->m_pModelRootObject, true);
	m_ppObjects[4]->m_pAnimationController = new CAnimationController(1, pEvilBearModel->m_pAnimationSets);
	m_ppObjects[4]->m_pAnimationController->SetTrackAnimationSet(0, 0);
	m_ppObjects[4]->m_pSkinningBoneTransforms = new CSkinningBoneTransforms(pd3dDevice, pd3dCommandList, pEvilBearModel);



	XMFLOAT3 Position;
	for (int i = 0; i < m_nObjects; ++i)
	{
		Position.x = Random(0.f, 500.f);
		Position.z = Random(0.f, 300.f);
		Position.y = pTerrain->GetHeight(Position.x, Position.z);
		m_ppObjects[i]->SetPosition(Position);
		m_ppObjects[i]->SetScale(10, 10, 10);

		m_ppObjects[i]->setID("<EvilBear>");
		auto iter = Context.find(m_ppObjects[i]->getID());
		if (iter != Context.end())
			m_ppObjects[i]->SetOOBB((*iter).second->m_xmf3Center, (*iter).second->m_xmf3Extent, XMFLOAT4(0, 0, 0, 1));

		dynamic_cast<CEvilBear*>(m_ppObjects[i])->Initialize_Shadow(pEvilBearModel, m_ppObjects[i]);
	}

	if (pEvilBearModel)
		delete pEvilBearModel;
}

void CSkinnedAnimationObjectShader::AnimateObjects(float fTimeElapsed, CCamera* pCamera, CPlayer* pPlayer)
{
	m_fElapsedTime = fTimeElapsed;
	
	for (int i = 0; i < m_nObjects; ++i)
	{
		if (m_ppObjects[i])
		{
			m_ppObjects[i]->Animate(m_fElapsedTime);
		}
	}
}

void CSkinnedAnimationObjectShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState)
{
	for (int i = 0; i < m_nObjects; ++i)
	{
		if (m_ppObjects[i])
		{
			m_ppObjects[i]->UpdateTransform(NULL);
			m_ppObjects[i]->Render(pd3dCommandList, 
				m_ppObjects[i]->GetIsHammer(), 
				m_ppObjects[i]->GetIsBomb(), 
				m_ppObjects[i]->GetBoolIce(), 
				m_ppObjects[i]->GetMaterialID(), 
				pCamera, nPipelineState);
		}
	}
}