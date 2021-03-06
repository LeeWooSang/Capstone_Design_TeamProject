#pragma once

#include "../Billboard.h"

class CSnow : public CBillboard
{
public:
	CSnow(int nMaterial);
	virtual ~CSnow();

	virtual void Animate(float fTimeElapsed, CCamera *pCamera = NULL);
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera, int nPipelineState, int nInstance);

	void SetOffsetX(float offset) { m_offsetX = offset; }
	float GetOffsetX()	const { return m_offsetX; }

	void SetOffsetZ(float offset) { m_offsetZ = offset; }
	float GetOffsetZ()	const { return m_offsetZ; }

	void SetPositionXZ(float x, float z);
	
	void setSpeed(float speed) { m_speed = speed; }

	void SetLookAt(XMFLOAT3& xmfTarget);

private:
	float						m_offsetX = 0.0f;
	float						m_offsetZ = 0.0f;
	float						m_speed = 0.f;
};