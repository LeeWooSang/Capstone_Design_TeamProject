#pragma once
#include "../Mesh.h"

//ī�޶���� �Ÿ��� ���� ����(������) ���� �����Ǵ� �޽�
class CLodMesh : public CStandardMesh
{
public:
	CLodMesh(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual ~CLodMesh();

public:

	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, int nSubSet, UINT lodLevel);

};