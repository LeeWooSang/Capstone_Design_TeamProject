#pragma once
#include "../GameObject.h"

class CLoadedModelInfo;
class CFoliageObject : public CGameObject
{
public:
	CFoliageObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature);
	virtual ~CFoliageObject();

	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, UINT lodlevel, CCamera *pCamera, int nPipelineState);
};