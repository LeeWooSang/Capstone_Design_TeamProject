#pragma once

#include "../UIShader.h"

class CTexture;
class CMaterial;
class CUI;
class CPlayer;
class CItemUIShader : public CUIShader
{
public:
	CItemUIShader();
	virtual ~CItemUIShader();

	virtual void CreateShader(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);
	virtual D3D12_SHADER_BYTECODE CreateVertexShader(int UIType);

	virtual void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature,
		const map<string, CTexture*>& Context, void* pContext);
	virtual void AnimateObjects(float fTimeElapsed, CCamera *pCamera, CPlayer* pPlayer = NULL);
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera, int nPipelineState);

private:
	enum UIType { ItemBoxUI, NormalItemUI, SpecialItemUI,SubSpecialItemUI };

	CPlayer* m_pPlayer{ nullptr };
};