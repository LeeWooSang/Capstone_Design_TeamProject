#pragma once
#include "../Mesh.h"

//ī�޶���� �Ÿ��� ���� ����(������) ���� �����Ǵ� �޽�
class CLODMesh : public CStandardMesh
{
public:
	CLODMesh(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual ~CLODMesh();

	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, int nSubSet, UINT lodLevel);
};