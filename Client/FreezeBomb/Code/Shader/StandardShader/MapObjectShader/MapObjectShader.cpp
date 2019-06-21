#include "../../../Stdafx/Stdafx.h"
#include "MapObjectShader.h"
#include "../../../GameObject/Surrounding/Surrounding.h"
#include "../../../GameObject/Terrain/Terrain.h"
#include "../../../ResourceManager/ResourceManager.h"
#include "../../../FrameTransform/FrameTransform.h"

CMapObjectsShader::CMapObjectsShader()
{
}

CMapObjectsShader::~CMapObjectsShader()
{
}

void CMapObjectsShader::BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature,
	const map<string, CLoadedModelInfo*>& ModelMap, const multimap<string, MapObjectInfo*>& MapObjectInfo, const map<string, Bounds*>& BoundMap, void* pContext)
{
	for (auto iter = ModelMap.begin(); iter != ModelMap.end(); ++iter)
	{
		string name = (*iter).first;
		// multimap �����̳ʿ��� ���� Ű�� ���� ������ ã�� ��, ����ϴ� ����
		for (auto iter2 = MapObjectInfo.lower_bound(name); iter2 != MapObjectInfo.upper_bound(name); ++iter2)
		{
			CSurrounding* pSurrounding = new CSurrounding(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
			pSurrounding->m_pFrameTransform = new CFrameTransform(pd3dDevice, pd3dCommandList, (*iter).second);

			pSurrounding->SetChild((*iter).second->m_pModelRootObject, true);
			pSurrounding->SetPosition((*iter2).second->m_Position);
			pSurrounding->SetLookVector((*iter2).second->m_Look);
			pSurrounding->SetUpVector((*iter2).second->m_Up);
			pSurrounding->SetRightVector((*iter2).second->m_Right);

			if (name != "PondSquare")
				pSurrounding->Initialize_Shadow((*iter).second, pSurrounding);

			auto iter3 = BoundMap.find(name);
			if (iter3 != BoundMap.end())
			{
				if (name == "SM_Deer")
				{
					pSurrounding->SetOOBB(Vector3::Add((*iter3).second->m_xmf3Center,XMFLOAT3(0.0f,0.0f,-3.5f)), Vector3::Multiply((*iter3).second->m_xmf3Extent, XMFLOAT3(1.0f,1.0f,3.5f)), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
				}
				else 
				{
					pSurrounding->SetOOBB((*iter3).second->m_xmf3Center, Vector3::Multiply((*iter3).second->m_xmf3Extent, (*iter).second->m_pModelRootObject->m_xmf3Scale), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
				}
			}

			m_SurroundingList.emplace_back(pSurrounding);
		}
	}
}

void CMapObjectsShader::AnimateObjects(float fTimeElapsed, CCamera* pCamera, CPlayer* pPlayer)
{
	m_fElapsedTime += fTimeElapsed;
}

void CMapObjectsShader::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera, int nPipelineState)
{
	for (auto iter = m_SurroundingList.begin(); iter != m_SurroundingList.end(); ++iter)
	{
		(*iter)->Animate(m_fElapsedTime);
		(*iter)->UpdateTransform(nullptr);
		(*iter)->Render(pd3dCommandList, pCamera, nPipelineState);
	}
}

void CMapObjectsShader::ReleaseObjects()
{
	for (auto iter = m_SurroundingList.begin(); iter != m_SurroundingList.end(); )
	{
		(*iter)->Release();
		iter = m_SurroundingList.erase(iter);
	}
	m_SurroundingList.clear();
}

void CMapObjectsShader::ReleaseUploadBuffers()
{
	for (auto iter = m_SurroundingList.begin(); iter != m_SurroundingList.end(); ++iter)
		(*iter)->ReleaseUploadBuffers();
}
D3D12_RASTERIZER_DESC CMapObjectsShader::CreateRasterizerState()
{
	D3D12_RASTERIZER_DESC d3dRasterizerDesc;
	::ZeroMemory(&d3dRasterizerDesc, sizeof(D3D12_RASTERIZER_DESC));
	//	d3dRasterizerDesc.FillMode = D3D12_FILL_MODE_WIREFRAME;
	d3dRasterizerDesc.FillMode = D3D12_FILL_MODE_WIREFRAME;
	d3dRasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	d3dRasterizerDesc.FrontCounterClockwise = FALSE;
	d3dRasterizerDesc.DepthBias = 0;
	d3dRasterizerDesc.DepthBiasClamp = 0.0f;
	d3dRasterizerDesc.SlopeScaledDepthBias = 0.0f;
	d3dRasterizerDesc.DepthClipEnable = TRUE;
	d3dRasterizerDesc.MultisampleEnable = FALSE;
	d3dRasterizerDesc.AntialiasedLineEnable = FALSE;
	d3dRasterizerDesc.ForcedSampleCount = 0;
	d3dRasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	return(d3dRasterizerDesc);
}