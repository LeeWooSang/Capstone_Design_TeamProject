#pragma once

#include "../UIShader.h"

class CTexture;
class CUI;
class CScene;
class CMenuUIShader : public CUIShader
{
public:
	CMenuUIShader();
	virtual ~CMenuUIShader();

	virtual void CreateShader(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);
	virtual D3D12_SHADER_BYTECODE CreateVertexShader(int UIType);

	virtual void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature,
		const map<string, CTexture*>& Context, void* pContext);
	virtual void AnimateObjects(float fTimeElapsed, CCamera *pCamera, CPlayer* pPlayer = NULL);
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera, int nPipelineState);
	virtual void ReleaseObjects();

	bool getIsRender()	const { return m_IsRender; }
	void setIsRender(bool value) { m_IsRender = value; }

	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseShaderVariables();

	bool UICollisionCheck(XMFLOAT2& mousePos, XMFLOAT2& minUIPos, XMFLOAT2& maxUIPos);

	enum MESSAGE_TPYE { MOUSE, KEYBOARD };
	void ProcessMessage(bool isMouse, UINT message, XMFLOAT2& mousePos);
	void ProcessMouseMessage(UINT message, XMFLOAT2& mousePos);
	void ProcessKeyBoardMessage();
	void ChangeState();
	void MenuUIClear();

private:
	enum MENU_TYPE { None = - 1, MenuBoard, Menu_ICON, Menu_Option, Menu_GameOver, Menu_Sound, Menu_Cartoon };
	bool m_IsRender = false;
	static byte m_MenuState;

	struct MenuUIInfo
	{
		XMFLOAT4 m_MenuBoard_MinMaxPos;
		XMFLOAT4 m_MenuICON_MinMaxPos;
		XMFLOAT4 m_MenuBoard_MenuICON_UV;

		XMFLOAT4 m_MenuOption_MinMaxPos;
		XMFLOAT4 m_MenuGameOver_MinMaxPos;
		XMFLOAT4 m_MenuOption_MenuGameOver_UV;

		XMFLOAT4 m_MenuSound_MinMaxPos;
		XMFLOAT4 m_MenuCartoon_MinMaxPos;
		XMFLOAT4 m_MenuSound_MenuCartoon_UV;
	};

	struct CB_UI
	{
		XMFLOAT4 m_MenuBoard_MinMaxPos;
		XMFLOAT4 m_MenuICON_MinMaxPos;
		XMFLOAT4 m_MenuBoard_MenuICON_UV;

		XMFLOAT4 m_MenuOption_MinMaxPos;
		XMFLOAT4 m_MenuGameOver_MinMaxPos;
		XMFLOAT4 m_MenuOption_MenuGameOver_UV;

		XMFLOAT4 m_MenuSound_MinMaxPos;
		XMFLOAT4 m_MenuCartoon_MinMaxPos;
		XMFLOAT4 m_MenuSound_MenuCartoon_UV;
	};

	MenuUIInfo m_MenuInfo;

	ID3D12Resource*		m_pd3dUIData{ nullptr };
	CB_UI*						m_pMappedUIData{ nullptr };
};