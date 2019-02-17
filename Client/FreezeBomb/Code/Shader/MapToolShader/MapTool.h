#pragma once
#include "../StandardShader/StandardShader.h"


class CMapToolShader : public CStandardShader
{
public:
	CMapToolShader();
	virtual ~CMapToolShader();

	virtual void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, void *pContext = NULL);
	//virtual void AnimateObjects(float fTimeElapsed)
	virtual void ReleaseObjects();
	
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera);
	

	void SortDescByName();	//������Ʈ �̸��� ������������ ���� (������ �̸��� ��� ���� ���Ҽ� ����)

	void InsertObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList,CPlayer *pPlayer,const string& model);
	void DeleteObject();
	void MakeMapFile();
	void MakeMapBinaryFile();

	int GetDeerModelCount() { return m_nDeerModelCount; }
	int GetPineTreeModelCount() { return m_nPineTreeModelCount; }
	int GetDeadTreeModelCount() { return m_nDeadTreeModelCount; }
	int GetBigRockModelCount() { return m_nBigRockModelCount; }
	int GetSmallRockModelCount() { return m_nSmallRockModelCount; }


	int GetCurrDeadTreeIndex() { return m_nCurrDeadTreeModelIndex; }
	int GetCurrPineTreeIndex() { return m_nCurrPineTreeModelIndex; }
	int GetCurrBigRockIndex() { return m_nCurrBigRockModelIndex; }
	int GetCurrSmallRockIndex() { return m_nCurrSmallRockModelIndex; }


	void SetCurrDeadTreeIndex(int index) { m_nCurrDeadTreeModelIndex = index; }
	void SetCurrPineTreeIndex(int index) { m_nCurrPineTreeModelIndex = index; }
	void SetCurrBigRockIndex(int index) { m_nCurrBigRockModelIndex = index; }
	void SetCurrSmallRockIndex(int index) { m_nCurrSmallRockModelIndex = index; }

protected:
	CTerrain*			m_pTerrain;
	map<string, CLoadedModelInfo*> m_ModelsList; // �𵨵��� �����ϴ� �� 

	vector<pair<string,CGameObject*>> m_Objects;			//���� ���Ͽ� �� ������Ʈ;

	int m_nObjects;

	int m_nDeadTreeModelCount = 0;
	int m_nPineTreeModelCount = 0;
	int m_nSmallRockModelCount = 0;
	int m_nBigRockModelCount = 0;
	int m_nDeerModelCount = 0;


	int m_nCurrDeadTreeModelIndex = 0;
	int m_nCurrPineTreeModelIndex = 0;
	int m_nCurrBigRockModelIndex = 0;
	int m_nCurrSmallRockModelIndex = 0;

};

