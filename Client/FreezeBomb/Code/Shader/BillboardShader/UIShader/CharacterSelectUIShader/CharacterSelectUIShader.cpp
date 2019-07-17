#include "../../../../Stdafx/Stdafx.h"
#include "CharacterSelectUIShader.h"
#include "../../../../Texture/Texture.h"
#include "../../../../Material/Material.h"
#include "../../../../Mesh/BillboardMesh/BillboardMesh.h"
#include "../../../../GameObject/Billboard/UI/UI.h"
#include "../../../../SoundSystem/SoundSystem.h"
#include "../../../../Scene/LobbyScene/LobbyScene.h"
#include "../../../../GameFramework/GameFramework.h"

bool CCharacterSelectUIShader::m_IsReady = false;
char CCharacterSelectUIShader::m_ChoiceCharacter = NONE;

extern byte g_PlayerCharacter;

CCharacterSelectUIShader::CCharacterSelectUIShader()
{
}

CCharacterSelectUIShader::~CCharacterSelectUIShader()
{
}

void CCharacterSelectUIShader::CreateShader(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature)
{
	m_nPipelineStates = 1;
	m_ppd3dPipelineStates = new ID3D12PipelineState*[m_nPipelineStates];

	for (int i = 0; i < m_nPipelineStates; ++i)
	{
		::ZeroMemory(&m_d3dPipelineStateDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
		m_d3dPipelineStateDesc.pRootSignature = pd3dGraphicsRootSignature;
		m_d3dPipelineStateDesc.VS = CreateVertexShader();
		m_d3dPipelineStateDesc.PS = CreatePixelShader();
		m_d3dPipelineStateDesc.RasterizerState = CreateRasterizerState();
		m_d3dPipelineStateDesc.BlendState = CreateBlendState();
		m_d3dPipelineStateDesc.DepthStencilState = CreateDepthStencilState();
		m_d3dPipelineStateDesc.InputLayout = CreateInputLayout();
		m_d3dPipelineStateDesc.SampleMask = UINT_MAX;
		m_d3dPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		m_d3dPipelineStateDesc.NumRenderTargets = 1;
		m_d3dPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		m_d3dPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
		m_d3dPipelineStateDesc.SampleDesc.Count = 1;
		m_d3dPipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

		HRESULT hResult = pd3dDevice->CreateGraphicsPipelineState(&m_d3dPipelineStateDesc, __uuidof(ID3D12PipelineState), (void **)&m_ppd3dPipelineStates[i]);
	}

	if (m_pd3dVertexShaderBlob) m_pd3dVertexShaderBlob->Release();
	if (m_pd3dPixelShaderBlob) m_pd3dPixelShaderBlob->Release();

	if (m_d3dPipelineStateDesc.InputLayout.pInputElementDescs) delete[] m_d3dPipelineStateDesc.InputLayout.pInputElementDescs;
}

void CCharacterSelectUIShader::CreateCbvSrvDescriptorHeaps(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, int nConstantBufferViews, int nShaderResourceViews)
{
	//���ҽ��� ����Ϸ��� �並 �������Ѵ�. 
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	d3dDescriptorHeapDesc.NumDescriptors = nConstantBufferViews + nShaderResourceViews; //CBVs + SRVs 
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	d3dDescriptorHeapDesc.NodeMask = 0;

	//������ ��: ���ҽ��� �����ϴ� �����ڵ��� �����ϴ� �������� �޸� ����(�迭)
	pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void **)&m_pd3dCbvSrvDescriptorHeap);

	//����̽�(�����)���� ������ ������ �޸� ũ�Ⱑ�ٸ�(32~64����Ʈ)
	// ������ ���� ����Ϸ��� ������ �迭�� ������ ũ�⸦ �˾ƾ� ��.
	m_d3dCbvCPUDescriptorStartHandle = m_pd3dCbvSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_d3dCbvGPUDescriptorStartHandle = m_pd3dCbvSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	m_d3dSrvCPUDescriptorStartHandle.ptr = m_d3dCbvCPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * nConstantBufferViews);
	m_d3dSrvGPUDescriptorStartHandle.ptr = m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * nConstantBufferViews);
}

void CCharacterSelectUIShader::CreateConstantBufferViews(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, int nConstantBufferViews, ID3D12Resource *pd3dConstantBuffers, UINT nStride)
{
	D3D12_GPU_VIRTUAL_ADDRESS d3dGpuVirtualAddress = pd3dConstantBuffers->GetGPUVirtualAddress();
	D3D12_CONSTANT_BUFFER_VIEW_DESC d3dCBVDesc;
	d3dCBVDesc.SizeInBytes = nStride;
	for (int j = 0; j < nConstantBufferViews; j++)
	{
		d3dCBVDesc.BufferLocation = d3dGpuVirtualAddress + (nStride * j);
		D3D12_CPU_DESCRIPTOR_HANDLE d3dCbvCPUDescriptorHandle;
		d3dCbvCPUDescriptorHandle.ptr = m_d3dCbvCPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * j);
		pd3dDevice->CreateConstantBufferView(&d3dCBVDesc, d3dCbvCPUDescriptorHandle);
	}
}

void CCharacterSelectUIShader::CreateShaderResourceViews(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CTexture *pTexture, UINT nRootParameter, bool bAutoIncrement)
{
	int nTextures = pTexture->GetTextures();
	int nTextureType = pTexture->GetTextureType();
	for (int i = 0; i < nTextures; i++)
	{
		ID3D12Resource *pShaderResource = pTexture->GetTexture(i);				//�ؽ�ó ���ҽ��� �ϳ� ���´�.
		D3D12_RESOURCE_DESC d3dResourceDesc = pShaderResource->GetDesc();		//�� ���ҽ��鿡 ���� ������ ���´�.
		D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc = GetShaderResourceViewDesc(d3dResourceDesc, nTextureType);  //�ؽ�ó�� ���� ������ �並 �����Ѵ�.
		pd3dDevice->CreateShaderResourceView(pShaderResource, &d3dShaderResourceViewDesc, m_d3dSrvCPUDescriptorStartHandle);		//cpu�� �����Ҽ��ִ� srv�並 �����.
		m_d3dSrvCPUDescriptorStartHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;

		//srv�並 
		pTexture->SetRootArgument(i, (bAutoIncrement) ? (nRootParameter + i) : nRootParameter, m_d3dSrvGPUDescriptorStartHandle);
		m_d3dSrvGPUDescriptorStartHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
	}
}

D3D12_SHADER_BYTECODE CCharacterSelectUIShader::CreateVertexShader()
{
	return(CShader::CompileShaderFromFile(L"../Code/Shader/HLSL/UI.hlsl", "VSCharacterSelect", "vs_5_1", &m_pd3dVertexShaderBlob));
}

D3D12_SHADER_BYTECODE CCharacterSelectUIShader::CreatePixelShader()
{
	return(CShader::CompileShaderFromFile(L"../Code/Shader/HLSL/UI.hlsl", "PSCharacterSelect", "ps_5_1", &m_pd3dVertexShaderBlob));
}

void CCharacterSelectUIShader::BuildObjects(ID3D12Device *pd3dDevice,ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, void *pContext)
{
	m_nObjects = 9;

	m_ppTextures = new CTexture*[m_nObjects];

	m_ppTextures[PINK] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	m_ppTextures[PINK]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"../Resource/Textures/Lobby/Lobby_Character_Select1.dds", 0);
	
	m_ppTextures[BROWN] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	m_ppTextures[BROWN]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"../Resource/Textures/Lobby/Lobby_Character_Select2.dds", 0);

	m_ppTextures[WHITE] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	m_ppTextures[WHITE]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"../Resource/Textures/Lobby/Lobby_Character_Select3.dds", 0);
	
	m_ppTextures[BLACK] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	m_ppTextures[BLACK]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"../Resource/Textures/Lobby/Lobby_Character_Select4.dds", 0);
	
	m_ppTextures[BLUE] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	m_ppTextures[BLUE]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"../Resource/Textures/Lobby/Lobby_Character_Select5.dds", 0);
	
	m_ppTextures[PANDA] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	m_ppTextures[PANDA]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"../Resource/Textures/Lobby/Lobby_Character_Select6.dds", 0);
	
	m_ppTextures[BASE] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	m_ppTextures[BASE]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"../Resource/Textures/Lobby/Lobby_Base.dds", 0);

	m_ppTextures[READY] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	m_ppTextures[READY]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"../Resource/Textures/Lobby/Lobby_Ready.dds", 0);

	m_ppTextures[QUIT] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	m_ppTextures[QUIT]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"../Resource/Textures/Lobby/Lobby_Quit.dds", 0);

	CBillboardMesh* pSelectMesh = new CBillboardMesh(pd3dDevice, pd3dCommandList, 20.f, 20.f, 0.0f, 0.0f, 0.0f, 0.0f);
	CreateCbvSrvDescriptorHeaps(pd3dDevice, pd3dCommandList, 0, m_nObjects);

	int index = 0;
	// minX, minY, maxX, maxY
	XMFLOAT4* minmaxPos = new XMFLOAT4[m_nObjects];

	// PINK ~ PANDA
	minmaxPos[index++] = { -0.928f, -0.585f, -0.74f, -0.345f };
	minmaxPos[index++] = { -0.69f, -0.585f , -0.515f, -0.345f };
	minmaxPos[index++] = { -0.455f, -0.585f, -0.28f, -0.345f };
	minmaxPos[index++] = { -0.92f, -0.88f, -0.748f, -0.645f };
	minmaxPos[index++] = { -0.69f, -0.88f, -0.515f, -0.645f };
	minmaxPos[index++] = { -0.458f, -0.88f, -0.28f, -0.645f };
	// BASE
	minmaxPos[index++] = { 0.f, 0.f, 0.f, 0.f };
	// READY
	minmaxPos[index++] = { 0.426f, -0.95f, 0.978f, -0.775f };
	// QUIT
	minmaxPos[index++] = { 0.736f, 0.835f, 0.965f, 0.973f };

	for (int i = 0; i < m_nObjects; ++i)
	{
		CreateShaderResourceViews(pd3dDevice, pd3dCommandList, m_ppTextures[i], 0, false);
		CMaterial* pMaterial = new CMaterial(1);
		pMaterial->SetTexture(m_ppTextures[i], 0);

		CUI* pUI = new CUI(1);
		pUI->SetMesh(pSelectMesh);
		pUI->SetMaterial(0, pMaterial);

		pUI->SetUIMinPos(XMFLOAT2(minmaxPos[i].x, minmaxPos[i].y));
		pUI->SetUIMaxPos(XMFLOAT2(minmaxPos[i].z, minmaxPos[i].w));

		m_UIMap.emplace(i, pUI);
	}

	delete[] minmaxPos;
	
}

void CCharacterSelectUIShader::CallbackMouse(UINT nMessgeID, float mouseX, float mouseY)
{
	for (auto iter = m_UIMap.begin(); iter != m_UIMap.end(); ++iter)
	{
		if ((*iter).first == LOBBY_CHARACTERSEL::BASE)	continue;

		XMFLOAT2 min = reinterpret_cast<CUI*>((*iter).second)->GetUIMinPos();
		XMFLOAT2 max = reinterpret_cast<CUI*>((*iter).second)->GetUIMaxPos();

		if (UICollisionCheck(XMFLOAT2(mouseX, mouseY), min, max) == false)	continue;
		
		if (m_IsReady == false)
		{
			// �浹�� ���� �ؽ��ķ� ����
			m_RenderingTexture = (*iter).first;
		}

		if (nMessgeID == WM_LBUTTONUP)
		{
			ClickInteraction((*iter).first);
			m_MouseState = MOUSE_STATE::MOUSE_CLICK;
			CSoundSystem::PlayingSound(CSoundSystem::CLICK);
			return;
		}

		// BASE���ٰ� ĳ���͸� ����,
		if (m_MouseState == MOUSE_STATE::NONE)
		{
			m_MouseState = MOUSE_STATE::MOUSE_ON;
			CSoundSystem::PlayingSound(CSoundSystem::CHARACTER_SELECT);
			return;
		}

		// ��� ���콺�� �޴��� �÷����� ��,
		else if (m_MouseState == MOUSE_STATE::MOUSE_ON)
			return;
		
	}

	if (m_IsReady == false)
	{
		// �κ� �߿��� �ƹ��͵� �浹�� ���� ���� ��,
		m_MouseState = MOUSE_STATE::NONE;
		m_RenderingTexture = LOBBY_CHARACTERSEL::BASE;
	}
}

void CCharacterSelectUIShader::ClickInteraction(int click)
{
	switch (click)
	{
	case PINK: 
	case BROWN:
	case WHITE:
	case BLACK:
	case BLUE:
	case PANDA:
		if (m_IsReady == false)
		{
			m_ChoiceCharacter = static_cast<unsigned char>(click);
#ifdef _WITH_SERVER_
			// ���� ����ĳ���͸� �����ߴ��� ����
			Network::GetInstance()->SendChoiceCharacter(m_ChoiceCharacter);
#endif
		}
		break;

	case READY:
		if (m_ChoiceCharacter == NONE)
		{
			cout << "ĳ���Ͱ� ���õ��� �ʾҽ��ϴ�." << endl;
			break;
		}

		m_IsReady = !m_IsReady;
#ifdef _WITH_SERVER_
		if (m_IsReady == false)
		{
			Network::GetInstance()->SendNotReady();
			m_RenderingTexture = LOBBY_CHARACTERSEL::BASE;
		}
		else
		{
			g_PlayerCharacter = m_ChoiceCharacter;
			Network::GetInstance()->SendReady();
		}
#else
		if (m_IsReady == false)
			m_RenderingTexture = LOBBY_CHARACTERSEL::BASE;
		else
			g_PlayerCharacter = m_ChoiceCharacter;
#endif
		break;

	case QUIT:
		::PostQuitMessage(0);
		break;

	default:
		break;
	}
}

void CCharacterSelectUIShader::Render(ID3D12GraphicsCommandList *pd3dCommandList,int nPipelineState)
{
	if (m_pd3dCbvSrvDescriptorHeap) 
		pd3dCommandList->SetDescriptorHeaps(1, &m_pd3dCbvSrvDescriptorHeap);

	CUIShader::OnPrepareRender(pd3dCommandList, 0);
	
	auto iter = m_UIMap.find(m_RenderingTexture);
	if (iter != m_UIMap.end())
		(*iter).second->Render(pd3dCommandList, nPipelineState);
}

void CCharacterSelectUIShader::ReleaseObjects()
{
	if (m_pd3dCbvSrvDescriptorHeap)
		m_pd3dCbvSrvDescriptorHeap->Release();

	CUIShader::ReleaseObjects();

	delete[] m_ppTextures;
}

bool CCharacterSelectUIShader::UICollisionCheck(XMFLOAT2& mousePos, XMFLOAT2& minUIPos, XMFLOAT2& maxUIPos)
{
	if (mousePos.x < minUIPos.x)
		return false;
	else if (mousePos.x > maxUIPos.x)
		return false;
	else if (mousePos.y > maxUIPos.y)
		return false;
	else if (mousePos.y < minUIPos.y)
		return false;

	return true;
}
