#include "../../../Stdafx/Stdafx.h"
#include "ItemShader.h"
#include "../../../GameObject/Item/Item.h"
#include "../../../GameObject/Terrain/Terrain.h"
#include "../../../FrameTransform/FrameTransform.h"

extern unsigned char g_Round;

CItemShader::CItemShader()
{
}

CItemShader::~CItemShader()
{
}

void CItemShader::BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature,
	const map<string, CLoadedModelInfo*>& ModelMap, const unordered_map<unsigned char, RoundInfo>& RoundMapObjectInfo, const map<string, Bounds*>& BoundMap, void* pContext)
{
	unsigned char round = 0;

	for (auto iter = RoundMapObjectInfo.begin(); iter != RoundMapObjectInfo.end(); ++iter)
	{
		map<string, CItem*> itemList;
		int nNormalHammer = 0, nGoldHammer = 0, nGoldTimer = 0;
		auto iter2 = ModelMap.find("Hammer");
		if (iter2 != ModelMap.end())
		{
			string name = (*iter2).first;
			int index=0;
			// multimap 컨테이너에서 같은 키를 갖는 벨류를 찾을 때, 사용하는 루프
			for (auto iter3 = (*iter).second.lower_bound(name); iter3 != (*iter).second.upper_bound(name); ++iter3)
			{
				CItem* pItem = new CItem;
				
				pItem->m_pFrameTransform = new CFrameTransform(pd3dDevice, pd3dCommandList, (*iter2).second);

				pItem->SetChild((*iter2).second->m_pModelRootObject, true);
				XMFLOAT3 position = (*iter3).second->m_Position;
				pItem->SetPosition(position.x, position.y + 0.5, position.z);
				pItem->SetLookVector((*iter3).second->m_Look);
				pItem->SetUpVector((*iter3).second->m_Up);
				pItem->SetRightVector((*iter3).second->m_Right);
				pItem->Initialize_Shadow((*iter2).second, pItem);

#ifdef _WITH_SERVER_
				auto iter4 = BoundMap.find(name);
				if (iter4 != BoundMap.end())
				{
					pItem->SetOOBB((*iter4).second->m_xmf3Center, Vector3::Multiply((*iter4).second->m_xmf3Extent, XMFLOAT3(3.0f, 3.0f, 3.0f)), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
				}

				if(index++ < 2)
				{
					pItem->setItemType(CItem::GoldHammer);
					itemList.emplace("GoldHammer " + to_string(nGoldHammer++), pItem);
				}
				else
				{
					pItem->setItemType(CItem::NormalHammer);
					itemList.emplace("NormalHammer " + to_string(nNormalHammer++), pItem);
				}
				
#else
				// 황금망치, 일반망치 랜덤으로 결정
				switch (rand() % 2)
				{
				case CItem::NormalHammer:
					pItem->setItemType(CItem::NormalHammer);
					//m_ItemMap.emplace("NormalHammer" + to_string(nNormalHammer++), pItem);
					itemList.emplace("NormalHammer " + to_string(nNormalHammer++), pItem);
					break;

				case CItem::GoldHammer:
					pItem->setItemType(CItem::GoldHammer);
					//m_ItemMap.emplace("GoldHammer" + to_string(nGoldHammer++), pItem);
					itemList.emplace("GoldHammer " + to_string(nGoldHammer++), pItem);
					break;
				}
#endif
			}
		}

		iter2 = ModelMap.find("GoldTimer");
		if (iter2 != ModelMap.end())
		{
			string name = (*iter2).first;
			// multimap 컨테이너에서 같은 키를 갖는 벨류를 찾을 때, 사용하는 루프
			for (auto iter3 = (*iter).second.lower_bound(name); iter3 != (*iter).second.upper_bound(name); ++iter3)
			{
				CItem* pItem = new CItem;
				pItem->m_pFrameTransform = new CFrameTransform(pd3dDevice, pd3dCommandList, (*iter2).second);

				pItem->SetChild((*iter2).second->m_pModelRootObject, true);
				XMFLOAT3 position = (*iter3).second->m_Position;
				pItem->SetPosition(position.x, position.y + 0.5, position.z);
				pItem->SetLookVector((*iter3).second->m_Look);
				pItem->SetUpVector((*iter3).second->m_Up);
				pItem->SetRightVector((*iter3).second->m_Right);
				pItem->Initialize_Shadow((*iter2).second, pItem);
#ifdef _WITH_SERVER_
				auto iter4 = BoundMap.find("Pocket_Watch");
				if (iter4 != BoundMap.end()) 
				{
					pItem->SetOOBB((*iter4).second->m_xmf3Center, XMFLOAT3(3.0f, 3.0f, 3.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
				}
#endif
				pItem->setItemType(CItem::GoldTimer);
				itemList.emplace("GoldTimer " + to_string(nGoldTimer++), pItem);

			}
		}
		m_ItemList.emplace(round++, itemList);
	}
}

void CItemShader::AnimateObjects(float fTimeElapsed, CCamera* pCamera, CPlayer* pPlayer)
{
	m_fElapsedTime += fTimeElapsed;
}

void CItemShader::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera, int nPipelineState)
{
	auto iter = m_ItemList.find(g_Round);
	if (iter != m_ItemList.end())
	{
		for (auto iter2 = (*iter).second.begin(); iter2 != (*iter).second.end(); ++iter2)
		{
			(*iter2).second->Animate(m_fElapsedTime);
			(*iter2).second->UpdateTransform(nullptr);
			(*iter2).second->Render(pd3dCommandList, pCamera, nPipelineState);
		}
	}
}

void CItemShader::ReleaseObjects()
{
	for (auto iter = m_ItemList.begin(); iter != m_ItemList.end(); )
	{
		for (auto iter2 = (*iter).second.begin(); iter2 != (*iter).second.end(); )
		{
			(*iter2).second->Release();
			iter2 = (*iter).second.erase(iter2);
		}
		(*iter).second.clear();
		iter = m_ItemList.erase(iter);
	}
	m_ItemList.clear();

	//for (auto iter = m_RemovedItemList.begin(); iter != m_RemovedItemList.end(); )
	//{
	//	for (auto iter2 = (*iter).second.begin(); iter2 != (*iter).second.end();)
	//	{
	//		(*iter2).second->Release();
	//		iter2 = (*iter).second.erase(iter2);
	//	}
	//	(*iter).second.clear();
	//	iter = m_RemovedItemList.erase(iter);
	//}
	//m_RemovedItemList.clear();

	for (auto iter = m_RemovedItemList.begin(); iter != m_RemovedItemList.end(); )
	{
		(*iter)->Release();
		iter = m_RemovedItemList.erase(iter);
	}
	m_RemovedItemList.clear();
}

void CItemShader::ReleaseUploadBuffers()
{
	for (auto iter = m_ItemList.begin(); iter != m_ItemList.end(); ++iter)
	{
		for (auto iter2 = (*iter).second.begin(); iter2 != (*iter).second.end(); ++iter2)
		{
			(*iter2).second->ReleaseUploadBuffers();
		}
	}
}

void CItemShader::ItemDelete(string key)
{
	auto iter = m_ItemList.find(g_Round);
	if (iter != m_ItemList.end())
	{
		auto iter2 = (*iter).second.find(key);
		if (iter2 != (*iter).second.end())
		{
			m_RemovedItemList.emplace_back((*iter2).second);
			iter2 = (*iter).second.erase(iter2);
		}
	}
}

void CItemShader::ItemClear()
{
	for (auto iter = m_ItemList.begin(); iter != m_ItemList.end(); )
	{
		for (auto iter2 = (*iter).second.begin(); iter2 != (*iter).second.end(); )
		{
			m_RemovedItemList.emplace_back((*iter2).second);
			iter2 = (*iter).second.erase(iter2);
		}
		(*iter).second.clear();
		iter = m_ItemList.erase(iter);
	}
	m_ItemList.clear();
}

const map<string, CItem*>& CItemShader::getItemList()	const
{
	auto iter = m_ItemList.find(g_Round);
	if (iter != m_ItemList.end())
		return (*iter).second;
}
