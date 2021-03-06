#include "../../../Stdafx/Stdafx.h"
#include "LoginScene.h" 
#include "../../../Shader/BillboardShader/UIShader/LoginShader/IDShader.h"
#include "../../../InputSystem/IDInputSystem/IDInputSystem.h"
#include "../../../Network/Network.h"

#ifdef _WITH_SERVER_
#ifdef _WITH_DIRECT2D_

CLoginScene::CLoginScene()
{
}
CLoginScene::~CLoginScene()
{
}

ID3D12RootSignature *CLoginScene::CreateGraphicsRootSignature(ID3D12Device* pd3dDevice)
{
	ID3D12RootSignature *pd3dGraphicsRootSignature = NULL;

	D3D12_DESCRIPTOR_RANGE pd3dDescriptorRanges[1];
	pd3dDescriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[0].NumDescriptors = 1;
	pd3dDescriptorRanges[0].BaseShaderRegister = 21; // t21: LoginSceneTexture
	pd3dDescriptorRanges[0].RegisterSpace = 0;
	pd3dDescriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	
	D3D12_ROOT_PARAMETER pd3dRootParameters[1];
	pd3dRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[0].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[0].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[0]);
	pd3dRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_STATIC_SAMPLER_DESC pd3dSamplerDescs[1];

	pd3dSamplerDescs[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	pd3dSamplerDescs[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	pd3dSamplerDescs[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	pd3dSamplerDescs[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	pd3dSamplerDescs[0].MipLODBias = 0;
	pd3dSamplerDescs[0].MaxAnisotropy = 1;
	pd3dSamplerDescs[0].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	pd3dSamplerDescs[0].MinLOD = 0;
	pd3dSamplerDescs[0].MaxLOD = D3D12_FLOAT32_MAX;
	pd3dSamplerDescs[0].ShaderRegister = 3;
	pd3dSamplerDescs[0].RegisterSpace = 0;
	pd3dSamplerDescs[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
	::ZeroMemory(&d3dRootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	d3dRootSignatureDesc.NumParameters = _countof(pd3dRootParameters);
	d3dRootSignatureDesc.pParameters = pd3dRootParameters;
	d3dRootSignatureDesc.NumStaticSamplers = _countof(pd3dSamplerDescs);
	d3dRootSignatureDesc.pStaticSamplers = pd3dSamplerDescs;
	d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;

	ID3DBlob *pd3dSignatureBlob = NULL;
	ID3DBlob *pd3dErrorBlob = NULL;
	D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pd3dSignatureBlob, &pd3dErrorBlob);
	pd3dDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(), pd3dSignatureBlob->GetBufferSize(), __uuidof(ID3D12RootSignature), (void **)&pd3dGraphicsRootSignature);
	if (pd3dSignatureBlob) pd3dSignatureBlob->Release();
	if (pd3dErrorBlob) pd3dErrorBlob->Release();

	return(pd3dGraphicsRootSignature);
}

void CLoginScene::ReleaseObjects()
{
	if (m_pInput)
	{
		m_pInput->Destroy();
		delete m_pInput;
	}

	if (m_pd3dGraphicsRootSignature)
		m_pd3dGraphicsRootSignature->Release();

	for (int i = 0; i < m_nShaders; i++)
	{
		m_ppShaders[i]->ReleaseShaderVariables();
		m_ppShaders[i]->ReleaseObjects();
		delete m_ppShaders[i];
	}
	delete[] m_ppShaders;
}

void CLoginScene::ReleaseUploadBuffers()
{
	for (int i = 0; i < m_nShaders; i++)
	{
		m_ppShaders[i]->ReleaseUploadBuffers();
	}
}

void CLoginScene::BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, IDWriteTextFormat* pFont, IDWriteTextLayout* pTextLayout, ID2D1SolidColorBrush* pFontColor)
{
	m_pd3dGraphicsRootSignature = CreateGraphicsRootSignature(pd3dDevice);

	int index = 0;

	m_nShaders = 1;
	m_ppShaders = new CShader*[m_nShaders];

	CIDShader *pIDShader = new CIDShader;
	pIDShader->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, nullptr);
	pIDShader->CreateShader(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
	m_ppShaders[index++] = pIDShader;

	m_pInput = new CIDInput;
	if (m_pInput)
		m_pInput->Initialize(pFont, pTextLayout, pFontColor);
}

XMFLOAT3 CLoginScene::ScreenPosition(int x, int y)
{
	D3D12_VIEWPORT d3dViewport = { 0, 0, FRAME_BUFFER_WIDTH , FRAME_BUFFER_HEIGHT, 0.0f, 1.0f };

	XMFLOAT3 screenPosition = XMFLOAT3(0.0f, 0.0f, 0.0f);

	screenPosition.x = (((2.0f * x) / d3dViewport.Width) - 1) / 1;
	screenPosition.y = - (((2.0f * y) / d3dViewport.Height) - 1) / 1;

	screenPosition.z = 0.0f;

	return screenPosition;
}

int CLoginScene::OnProcessingMouseMessage(HWND hWnd,UINT nMessageID,WPARAM wParam,LPARAM lParam)
{
	int mouseX = LOWORD(lParam);
	int mouseY = HIWORD(lParam);

	int ret = CIDShader::state::NO_SELECT;
	switch(nMessageID)
	{
		case WM_LBUTTONUP:
		{
			POINT ptCursorPos;
	
			GetCursorPos(&ptCursorPos);
			ScreenToClient(hWnd,&ptCursorPos);
					
			RECT rect{ 0,0 };
			GetClientRect(hWnd, &rect);
			ret = dynamic_cast<CIDShader*>(m_ppShaders[0])->DecideTextureByCursor(rect,ptCursorPos.x, ptCursorPos.y);
		
			if(ret == CIDShader::state::REQUEST_LOGIN)
			{
				m_bLogin = true;
				return ret;
			}
			break;
		}
	}
	return ret;
}

void CLoginScene::AnimateObjects(ID3D12GraphicsCommandList* pd3dCommandList, float elapsedTime)
{
	for (int i = 0; i < m_nShaders; ++i)
	{
		if (m_ppShaders[i])
		{
			m_ppShaders[i]->AnimateObjects(elapsedTime, nullptr, nullptr);
		}
	}
}

void CLoginScene::Render(ID3D12GraphicsCommandList *pd3dCommandList)
{
	if (m_pd3dGraphicsRootSignature) 
		pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);

	for(int i = 0; i < m_nShaders; ++i)
	{
		if(m_pInput->IsIDLength()>0)
		{
			m_ppShaders[i]->Render(pd3dCommandList, 0, true);
		}
		else
		{
			m_ppShaders[i]->Render(pd3dCommandList, 0, false);
		}
	}
}
#endif
#endif
