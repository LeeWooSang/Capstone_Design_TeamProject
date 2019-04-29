#include "../Stdafx/Stdafx.h"
#include "ShaderManager.h"
#include "../ResourceManager/ResourceManager.h"

#include "../Shader/Shader.h"
#include "../Shader/SkyBoxShader/SkyBoxShader.h"
#include "../Shader/TerrainShader/TerrainShader.h"

#include "../Shader/StandardShader/MapObjectShader/MapObjectShader.h"
#include "../Shader/StandardShader/FoliageShader/FoliageShader.h"
#include "../Shader/StandardShader/ItemShader/ItemShader.h"
#include "../Shader/StandardShader/SkinnedAnimationShader/SkinnedAnimationObjectShader/SkinnedAnimationObjectShader.h"

#include "../Shader/CubeParticleShader/CubeParticleShader.h"
#include "../Shader/BillboardShader/BombParticleShader/BombParticleShader.h"
#include "../Shader/BillboardShader/SnowShader/SnowShader.h"

#include "../Shader/BillboardShader/UIShader/TimerUIShader/TimerUIShader.h"
#include "../Shader/BillboardShader/UIShader/ItemUIShader/ItemUIShader.h"

//#include "../Shader/PostProcessShader/CartoonShader/SobelCartoonShader.h"

#include "../GameObject/Player/Player.h"

CShaderManager::CShaderManager()
{
}

CShaderManager::~CShaderManager()
{
}

void CShaderManager::Initialize(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature,const int& nPlayerCount)
{
	m_pResourceManager = new CResourceManager;
	m_pResourceManager->Initialize(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);

	m_nShaders = 9;

	//���� ����϶��� ���� ������Ʈ���� �׸��� �ʰ� �ϱ� ���� 
	// �׷��� ������忡�� �����ؼ� ��ġ�� ������Ʈ�鸸 �� �� �ִ�.

#ifdef _MAPTOOL_MODE_
	m_nShaders = m_nShaders - 1;
#endif
	//ī�������� �ؾߵ� ���̴� ����
	m_nPostShaders = m_nShaders - 2;

	m_ppShaders = new CShader*[m_nShaders];


	int index = 0;
	CSkyBoxShader* pSkyBoxShader = new CSkyBoxShader;
	pSkyBoxShader->CreateShader(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
	pSkyBoxShader->BuildObjects(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, m_pResourceManager->getTextureMap(), nullptr);
	m_ppShaders[index++] = pSkyBoxShader;
	m_ShaderMap.emplace("SkyBox", pSkyBoxShader);

	CTerrainShader* pTerrainShader = new CTerrainShader;
	pTerrainShader->CreateShader(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
	pTerrainShader->BuildObjects(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, m_pResourceManager->getTextureMap(), nullptr);
	m_ppShaders[index++] = pTerrainShader;
	m_ShaderMap.emplace("Terrain", pTerrainShader);

#ifndef _MAPTOOL_MODE_
	CMapObjectsShader *pMapShader = new CMapObjectsShader;
	pMapShader->BuildObjects(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature,
		m_pResourceManager->getModelMap(), m_pResourceManager->getMapObjectInfo(), m_pResourceManager->getBoundMap(), pTerrainShader->getTerrain());
	m_ppShaders[index++] = pMapShader;
	m_ShaderMap.emplace("MapObjects", pMapShader);
#endif

	//////Foliage�� �浹ó���� �ʿ� ����.. ���� Bound �ڽ� �ʿ�  ����. �׸��ڵ� �ʿ����
	//CFoliageShader* pFoliageShader = new CFoliageShader;
	//pFoliageShader->CreateShader(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
	//pFoliageShader->BuildObjects(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, m_pResourceManager->getModelMap(),pTerrainShader->getTerrain());
	//m_ppShaders[index++] = pFoliageShader;
	//m_ShaderMap.emplace("Foliage", pFoliageShader);

	CItemShader* pItemShader = new CItemShader;
	//pItemShader->CreateShader(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
	pItemShader->BuildObjects(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, m_pResourceManager->getModelMap(), m_pResourceManager->getBoundMap(), pTerrainShader->getTerrain());
	m_ppShaders[index++] = pItemShader;
	m_ShaderMap.emplace("Item", pItemShader);

	/*
	CSobelCartoonShader *pCartoonShader = new CSobelCartoonShader;
	pCartoonShader->CreateGraphicsRootSignature(pd3dDevice);
	pCartoonShader->CreateShader(pd3dDevice, pCartoonShader->GetGraphicsRootSignature(), 1);
	pCartoonShader->BuildObjects(pd3dDevice, pd3dCommandList, m_pResourceManager->getTextureMap());
	*/

	CSkinnedAnimationObjectShader* pAnimationObjectShader = new CSkinnedAnimationObjectShader;
	//pAnimationObjectShader->CreateShader(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
	pAnimationObjectShader->BuildObjects(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, m_pResourceManager->getModelMap(), m_pResourceManager->getBoundMap(),nPlayerCount,pTerrainShader->getTerrain());
	m_ppShaders[index++] = pAnimationObjectShader;
	m_ShaderMap.emplace("������", pAnimationObjectShader);

	CBombParticleShader* pBombParticleShader = new CBombParticleShader;
	pBombParticleShader->CreateShader(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
	pBombParticleShader->BuildObjects(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, m_pResourceManager->getTextureMap(), nullptr);
	m_ppShaders[index++] = pBombParticleShader;
	m_ShaderMap.emplace("Bomb", pBombParticleShader);

	CCubeParticleShader* pCubeParticleShader = new CCubeParticleShader;
	pCubeParticleShader->CreateShader(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
	pCubeParticleShader->BuildObjects(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, m_pResourceManager->getTextureMap(), nullptr);
	m_ppShaders[index++] = pCubeParticleShader;
	m_ShaderMap.emplace("CubeParticle", pCubeParticleShader);

	CSnowShader * pSnowShader = new CSnowShader;
	pSnowShader->CreateShader(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
	pSnowShader->BuildObjects(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, m_pResourceManager->getTextureMap(), pTerrainShader->getTerrain());
	m_ppShaders[index++] = pSnowShader;
	m_ShaderMap.emplace("Snow", pSnowShader);

	/////////////////////////////////////////////////////////////////////////////////////////////////
	//�Ʒ� shader���� ī��ó���� �Ǹ� �ȵǴ� shader

	CTimerUIShader* pTimerUIShader = new CTimerUIShader;
	pTimerUIShader->CreateShader(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
	pTimerUIShader->BuildObjects(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, m_pResourceManager->getTextureMap(), nullptr);
	m_ppShaders[index++] = pTimerUIShader;
	m_ShaderMap.emplace("TimerUI", pTimerUIShader);

	CItemUIShader* pItemUIShader = new CItemUIShader;
	pItemUIShader->CreateShader(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
	pItemUIShader->BuildObjects(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, m_pResourceManager->getTextureMap(), nullptr);
	m_ppShaders[index++] = pItemUIShader;
	m_ShaderMap.emplace("ItemUI", pItemUIShader);
	

#ifndef _MAPTOOL_MODE_
	m_pResourceManager->ReleaseModel();
#endif
}

void CShaderManager::ReleaseObjects()
{
	if (m_ppShaders)
	{
		for (int i = 0; i < m_nShaders; i++)
		{
			m_ppShaders[i]->ReleaseShaderVariables();
			m_ppShaders[i]->ReleaseObjects();
			m_ppShaders[i]->Release();
		}
		delete[] m_ppShaders;
	}

#ifdef _MAPTOOL_MODE_
	m_pResourceManager->ReleaseModel();
#endif

	if (m_pResourceManager)
	{
		m_pResourceManager->Release();
		delete m_pResourceManager;
	}
}

void CShaderManager::ReleaseUploadBuffers()
{
	for (int i = 0; i < m_nShaders; i++)
		m_ppShaders[i]->ReleaseUploadBuffers();
}

void CShaderManager::AnimateObjects(float elapsedTime, CCamera* pCamera, CPlayer* pPlayer)
{
	m_pPlayer = pPlayer;

	for (int i = 0; i < m_nShaders; i++) {
		m_ppShaders[i]->AnimateObjects(elapsedTime, pCamera, pPlayer);
	}
}

//ī�� ������ ���� �ʰ� �׷����� ���̴� 
void CShaderManager::PostRender(ID3D12GraphicsCommandList* pd3dCommandList,float fTimeElapsed, CCamera* pCamera)
{
	for (int i = m_nPostShaders; i <m_nShaders; i++)
	{
		if (m_ppShaders[i]) 
			m_ppShaders[i]->Render(pd3dCommandList, pCamera, GameObject);
	}
	
}

void CShaderManager::PreRender(ID3D12GraphicsCommandList* pd3dCommandList,float fTimeElapsed, CCamera* pCamera)
{
	for (int i = 0; i < m_nPostShaders; i++)
	{
		if (m_ppShaders[i]) 
			m_ppShaders[i]->Render(pd3dCommandList, pCamera, GameObject);
	}

	if (m_pPlayer)
		m_pPlayer->setShaderManager(this);
}

void CShaderManager::ProcessCollision(XMFLOAT3& position)
{
	auto iter = m_ShaderMap.find("CubeParticle");
	if(iter!= m_ShaderMap.end())
		dynamic_cast<CCubeParticleShader*>((*iter).second)->SetParticleBlowUp(position);
}