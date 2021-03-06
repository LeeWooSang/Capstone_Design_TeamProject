#pragma once

#include "../UIShader.h"

class CTexture;
class CUI;
class CTimerUIShader : public CUIShader
{
public:
	CTimerUIShader();
	virtual ~CTimerUIShader();

	virtual void CreateShader(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);
	virtual D3D12_SHADER_BYTECODE CreateVertexShader(int UIType);

	virtual void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature,
		const map<string, CTexture*>& Context, void* pContext);
	virtual void AnimateObjects(float fTimeElapsed, CCamera *pCamera, CPlayer* pPlayer = NULL);
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera, int nPipelineState);
	virtual void ReleaseObjects();

	void TimerUpdate();



	static void setTimer(float timer);
	static float getTimer() { return m_Timer; }

	static void setReduceTimer(float value);

	static void ServerTimeAndSet(float timer) { m_Timer = timer; }

private:
	enum UIType { OneSec, TenSec, OneMin, Colon };

	map<int, CUI*>	m_OneSecUIMap;
	map<int, CUI*>	m_TenSecUIMap;
	map<int, CUI*>	m_OneMinUIMap;

	// �ʴ���
	static float m_Timer;

	

	int		m_Sec = 0;
	int		m_TenSec = 0;
	int		m_Min = 0;
	int		m_TenMin = 0;
};