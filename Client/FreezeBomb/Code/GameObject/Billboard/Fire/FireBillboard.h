#pragma once

#include "../Billboard.h"

struct CB_ANIMATIONCLIP;

class CFireBillboard : public CBillboard
{
public:
	CFireBillboard(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);
	virtual ~CFireBillboard();

	void SetLookAt(XMFLOAT3& xmfTarget);

public:
	virtual void Animate(float fTimeElapsed);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera *pCamera, int nPipelineStates = GameObject);
	
	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseShaderVariables();

	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);

private:
	UINT														m_AnimationClip = 0;

	struct CB_World
	{
		XMFLOAT4X4		m_World;
	};

	ID3D12Resource*		m_pd3dcbWorld{ nullptr };
	CB_World*				m_pcbMappedWorld{ nullptr };

	CB_ANIMATIONCLIP*							m_pcbMappedAnimationClip = NULL;
	ID3D12Resource*									m_pd3dcbAnimationClip = NULL;
};