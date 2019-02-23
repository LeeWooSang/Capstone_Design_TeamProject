#include "../../Stdafx/Stdafx.h"
#include "PlayerShadowShader.h"
#include "../../GameObject/GameObject.h"
#include "../../GameObject/Player/Player.h"
#include "../../Texture/Texture.h"

#ifdef CUBE
#include "../../Material/Material.h"
#include "../../Mesh/Mesh.h"
#else
#include "../../GameObject/EvilBear/EvilBear.h"

#endif

CPlayerShadowShader::CPlayerShadowShader()
{
}

CPlayerShadowShader::~CPlayerShadowShader()
{
	if (m_ppd3dPipelineStates)
	{
		for (int i = 0; i < m_nPipelineStates; ++i)
			if (m_ppd3dPipelineStates[i])
				m_ppd3dPipelineStates[i]->Release();
		delete[] m_ppd3dPipelineStates;
	}
}

void CPlayerShadowShader::CreateShader(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature)
{
	m_nPipelineStates = 2;
	m_ppd3dPipelineStates = new ID3D12PipelineState*[m_nPipelineStates];

	for (int i = 0; i < m_nPipelineStates; ++i)
	{
		::ZeroMemory(&m_d3dPipelineStateDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
		m_d3dPipelineStateDesc.pRootSignature = pd3dGraphicsRootSignature;
		m_d3dPipelineStateDesc.VS = CreateVertexShader(i);
		m_d3dPipelineStateDesc.PS = CreatePixelShader(i);
		m_d3dPipelineStateDesc.RasterizerState = CreateRasterizerState();
		m_d3dPipelineStateDesc.BlendState = CreateBlendState(i);
		m_d3dPipelineStateDesc.DepthStencilState = CreateDepthStencilState(i);
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

D3D12_SHADER_BYTECODE CPlayerShadowShader::CreateVertexShader(int Type)
{
	switch (Type)
	{
#ifdef CUBE
	case Cube:
		return(CShader::CompileShaderFromFile(L"../Code/Shader/HLSL/Shaders.hlsl", "VSIceCube", "vs_5_1", &m_pd3dVertexShaderBlob));
		break;
#else
	case Surrounding:
		return(CShader::CompileShaderFromFile(L"../Code/Shader/HLSL/Shaders.hlsl", "VSStandard", "vs_5_1", &m_pd3dVertexShaderBlob));
		break;
#endif
	case Shadow:
		//return(CShader::CompileShaderFromFile(L"../Code/Shader/HLSL/Shaders.hlsl", "VSAnimationShadow", "vs_5_1", &m_pd3dVertexShaderBlob));
		return(CShader::CompileShaderFromFile(L"../Code/Shader/HLSL/Shaders.hlsl", "VSShadow", "vs_5_1", &m_pd3dVertexShaderBlob));
		break;
	}
}

D3D12_SHADER_BYTECODE CPlayerShadowShader::CreatePixelShader(int Type)
{
	switch (Type)
	{
#ifdef CUBE
	case Cube:
		return(CShader::CompileShaderFromFile(L"../Code/Shader/HLSL/Shaders.hlsl", "PSIceCube", "ps_5_1", &m_pd3dPixelShaderBlob));
		break;
#else
	case Surrounding:
		return(CShader::CompileShaderFromFile(L"../Code/Shader/HLSL/Shaders.hlsl", "PSStandard", "ps_5_1", &m_pd3dPixelShaderBlob));
		break;
#endif
	case Shadow:
		//return(CShader::CompileShaderFromFile(L"../Code/Shader/HLSL/Shaders.hlsl", "PSAnimationShadow", "ps_5_1", &m_pd3dPixelShaderBlob));
		return(CShader::CompileShaderFromFile(L"../Code/Shader/HLSL/Shaders.hlsl", "PSShadow", "ps_5_1", &m_pd3dPixelShaderBlob));
		break;
	}
}

D3D12_INPUT_LAYOUT_DESC CPlayerShadowShader::CreateInputLayout()
{
	//UINT nInputElementDescs = 5;
	//D3D12_INPUT_ELEMENT_DESC *pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	//pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	//pd3dInputElementDescs[1] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	//pd3dInputElementDescs[2] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	//pd3dInputElementDescs[3] = { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 3, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	//pd3dInputElementDescs[4] = { "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 4, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	//D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	//d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
	//d3dInputLayoutDesc.NumElements = nInputElementDescs;

	//return(d3dInputLayoutDesc);

	UINT nInputElementDescs = 7;
	D3D12_INPUT_ELEMENT_DESC *pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[1] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[2] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[3] = { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 3, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[4] = { "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 4, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[5] = { "BONEINDEX", 0, DXGI_FORMAT_R32G32B32A32_UINT, 5, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[6] = { "BONEWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 6, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
	d3dInputLayoutDesc.NumElements = nInputElementDescs;

	return(d3dInputLayoutDesc);
}

D3D12_BLEND_DESC CPlayerShadowShader::CreateBlendState(int Type)
{
	D3D12_BLEND_DESC d3dBlendDesc;
	::ZeroMemory(&d3dBlendDesc, sizeof(D3D12_BLEND_DESC));
	switch (Type)
	{
	case Surrounding:
		d3dBlendDesc.AlphaToCoverageEnable = FALSE;
		d3dBlendDesc.IndependentBlendEnable = FALSE;
		d3dBlendDesc.RenderTarget[0].BlendEnable = FALSE;
		d3dBlendDesc.RenderTarget[0].LogicOpEnable = FALSE;
		d3dBlendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
		d3dBlendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
		d3dBlendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		d3dBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
		d3dBlendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
		d3dBlendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
		d3dBlendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
		d3dBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
		break;

	case Shadow:
		d3dBlendDesc.AlphaToCoverageEnable = true;
		d3dBlendDesc.IndependentBlendEnable = true;
		d3dBlendDesc.RenderTarget[0].BlendEnable = true;
		d3dBlendDesc.RenderTarget[0].LogicOpEnable = false;
		//d3dBlendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ZERO;
		d3dBlendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
		//d3dBlendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_SRC_COLOR;
		d3dBlendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
		d3dBlendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		d3dBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
		d3dBlendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
		d3dBlendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
		//d3dBlendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
		d3dBlendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_CLEAR;
		d3dBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
		break;
	}

	return(d3dBlendDesc);
}

D3D12_DEPTH_STENCIL_DESC CPlayerShadowShader::CreateDepthStencilState(int Type)
{
	D3D12_DEPTH_STENCIL_DESC d3dDepthStencilDesc;
	::ZeroMemory(&d3dDepthStencilDesc, sizeof(D3D12_DEPTH_STENCIL_DESC));

	switch (Type)
	{
	case Surrounding:
		//d3dDepthStencilDesc.DepthEnable = TRUE;
		d3dDepthStencilDesc.DepthEnable = false;
		d3dDepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
		d3dDepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
		d3dDepthStencilDesc.StencilEnable = FALSE;
		d3dDepthStencilDesc.StencilReadMask = 0x00;
		d3dDepthStencilDesc.StencilWriteMask = 0x00;
		d3dDepthStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
		d3dDepthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
		d3dDepthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
		d3dDepthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;
		d3dDepthStencilDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
		d3dDepthStencilDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
		d3dDepthStencilDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
		d3dDepthStencilDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;
		break;

	case Shadow:
		d3dDepthStencilDesc.DepthEnable = false;
		//d3dDepthStencilDesc.DepthEnable = true;
		d3dDepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
		d3dDepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
		//d3dDepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
		d3dDepthStencilDesc.StencilEnable = true;
		d3dDepthStencilDesc.StencilReadMask = 0xff;
		//d3dDepthStencilDesc.StencilReadMask = 0x02;
		d3dDepthStencilDesc.StencilWriteMask = 0xff;
		//d3dDepthStencilDesc.StencilWriteMask = 0x02;
		d3dDepthStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
		d3dDepthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
		d3dDepthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_INCR;
		//d3dDepthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_REPLACE;
		d3dDepthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_EQUAL;
		//d3dDepthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_NOT_EQUAL;

		d3dDepthStencilDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
		d3dDepthStencilDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
		d3dDepthStencilDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_INCR;
		//d3dDepthStencilDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_REPLACE;
		d3dDepthStencilDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_EQUAL;
		//d3dDepthStencilDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
		break;
	}

	return(d3dDepthStencilDesc);
}

void CPlayerShadowShader::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList, XMFLOAT4X4* pShadowWorld)
{
}

void CPlayerShadowShader::BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature,
	const map<string, CTexture*>& Context, void* pContext)
{
	//CTerrain* pTerrain = (CTerrain*)pContext;
	CPlayer* pPlayer = (CPlayer*)pContext;

	m_nObjects = 1;
	XMFLOAT3 Position(0, 0, 0);

#ifdef CUBE
	CCubeMeshTextured* pCubeMesh = new CCubeMeshTextured(pd3dDevice, pd3dCommandList, 4.f, 4.0f, 4.0f);

	CMaterial*pMaterial = new CMaterial(1);
	auto iter = Context.find("IceTexture");
	if (iter != Context.end())
		pMaterial->SetTexture((*iter).second, 0);

	for (int i = 0; i < m_nObjects; ++i)
	{
		CCubeObject* pCube = new CCubeObject(1);
		Position = XMFLOAT3(Random(10, 490), 5, Random(10, 290));
		pCube->SetPosition(Position);
		pCube->SetMesh(pCubeMesh);
		pCube->SetMaterial(0, pMaterial);
		m_ppObjects[i] = pCube;
	}

	for (int i = 0; i < m_nObjects; ++i)
	{
		CCubeObject* pCube = new CCubeObject(1);
		Position = XMFLOAT3(m_ppObjects[i]->GetPosition());
		pCube->SetPosition(Position);
		pCube->SetMesh(pCubeMesh);
		pCube->SetMaterial(0, pMaterial);
		m_ShadowObjectVector.emplace_back(pCube);
	}

#else
	CLoadedModelInfo* pEvilBear = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature,
		"../Resource/Models/EvilBear.bin", this, false);

	for (int i = 0; i < m_nObjects; ++i)
	{
		CEvilBear* pShadow = new CEvilBear(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, 0);
		Position = XMFLOAT3(pPlayer->GetPosition());
		pShadow->SetPosition(Position);
		pShadow->SetChild(pEvilBear->m_pModelRootObject, true);
		m_ShadowObjectVector.emplace_back(pShadow);
	}

	if (pEvilBear)
		delete pEvilBear;

#endif
}

void CPlayerShadowShader::OnPrepareRender(ID3D12GraphicsCommandList *pd3dCommandList, int nPipelineState)
{
	if (m_ppd3dPipelineStates)
		pd3dCommandList->SetPipelineState(m_ppd3dPipelineStates[nPipelineState]);
}

void CPlayerShadowShader::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera, CPlayer* pPlayer)
{
	for (auto iter = m_ShadowObjectVector.begin(); iter != m_ShadowObjectVector.end(); ++iter)
	{
		CPlayerShadowShader::OnPrepareRender(pd3dCommandList, Shadow);
		(*iter)->WorldUpdate(UpdateShadow(pPlayer));
		(*iter)->Render(pd3dCommandList, pCamera);
	}
}

void CPlayerShadowShader::ReleaseObjects()
{
	for (auto iter = m_ShadowObjectVector.begin(); iter != m_ShadowObjectVector.end();)
	{
		delete (*iter);
		iter = m_ShadowObjectVector.erase(iter);
	}
	m_ShadowObjectVector.clear();
}

void CPlayerShadowShader::ReleaseUploadBuffers()
{
	for (auto iter = m_ShadowObjectVector.begin(); iter != m_ShadowObjectVector.end(); ++iter)
		(*iter)->ReleaseUploadBuffers();
}

XMFLOAT4X4 CPlayerShadowShader::UpdateShadow(CPlayer* pPlayer)
{
	// �� ����
	// ���� ��ġ�� ��Ÿ���� 4D ����. ������ w- ������ 0.0f�̸� �������� �������� ������ ���⼺�� ��Ÿ���ϴ�. 1.0f�̸� �����Դϴ�.
	// Light�� w ���Ͱ� �׸����� ũ�⸦ ����?
	float x, y, z, w;
	x = 0;
	y = 1.;
	z = 0;
	w = 1.0;
	//XMFLOAT4 xmf4Light(x, y, z, w);
	//XMFLOAT4 xmf4Light(35.f, 35.f,35.f, 10.f);
	XMFLOAT4 xmf4Light(0.57735f, -0.57735f, 0.57735f, 0);

	// Plane�� w ���Ͱ� �׸����� y�� ������ �ش�.
	XMFLOAT4 xmf4Plane(0.f, 1.f, 0.f, 0.f);
	//XMFLOAT4 xmf4Plane(0.f, 1.f, 0.f, 0.f);

	// �׸��� ��� ����
	//XMMATRIX xmmtxPlane = XMMatrixShadow(XMLoadFloat4(&xmf4Plane), XMLoadFloat4(&xmf4Light));
	XMMATRIX xmmtxPlane = XMMatrixShadow(XMLoadFloat4(&xmf4Plane), -XMLoadFloat4(&xmf4Light));

	// �׸��� ��Ŀ� ��ü�� ��������� ���ؼ� �׸����� ��������� ����� �ش�.
	XMFLOAT4X4 ShadowWorld = Matrix4x4::Multiply(xmmtxPlane, pPlayer->m_xmf4x4World);

	//cout << ShadowWorld._41 << ", " << ShadowWorld._42 << ", " << ShadowWorld._43 << endl;
	return ShadowWorld;
}