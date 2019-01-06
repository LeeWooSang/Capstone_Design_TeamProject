//-----------------------------------------------------------------------------
// File: Shader.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Shader.h"
#include "DDSTextureLoader12.h"


#define TEXTURES 4

CShader::CShader()
{
	m_d3dSrvCPUDescriptorStartHandle.ptr = NULL;
	m_d3dSrvGPUDescriptorStartHandle.ptr = NULL;
}

CShader::~CShader()
{
	if (m_pd3dPipelineState) m_pd3dPipelineState->Release();
}

D3D12_SHADER_BYTECODE CShader::CreateVertexShader(ID3DBlob **ppd3dShaderBlob)
{
	D3D12_SHADER_BYTECODE d3dShaderByteCode;
	d3dShaderByteCode.BytecodeLength = 0;
	d3dShaderByteCode.pShaderBytecode = NULL;

	return(d3dShaderByteCode);
}

D3D12_SHADER_BYTECODE CShader::CreatePixelShader(ID3DBlob **ppd3dShaderBlob)
{
	D3D12_SHADER_BYTECODE d3dShaderByteCode;
	d3dShaderByteCode.BytecodeLength = 0;
	d3dShaderByteCode.pShaderBytecode = NULL;

	return(d3dShaderByteCode);
}

D3D12_SHADER_BYTECODE CShader::CompileShaderFromFile(WCHAR *pszFileName, LPCSTR pszShaderName, LPCSTR pszShaderProfile, ID3DBlob **ppd3dShaderBlob)
{
	UINT nCompileFlags = 0;
#if defined(_DEBUG)
	nCompileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	ID3DBlob *pd3dErrorBlob = NULL;
	::D3DCompileFromFile(pszFileName, NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, pszShaderName, pszShaderProfile, nCompileFlags, 0, ppd3dShaderBlob, NULL);
	//	char *pErrorString = (char *)pd3dErrorBlob->GetBufferPointer();

	D3D12_SHADER_BYTECODE d3dShaderByteCode;
	d3dShaderByteCode.BytecodeLength = (*ppd3dShaderBlob)->GetBufferSize();
	d3dShaderByteCode.pShaderBytecode = (*ppd3dShaderBlob)->GetBufferPointer();

	return(d3dShaderByteCode);
}

D3D12_INPUT_LAYOUT_DESC CShader::CreateInputLayout()
{
	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = NULL;
	d3dInputLayoutDesc.NumElements = 0;

	return(d3dInputLayoutDesc);
}

D3D12_RASTERIZER_DESC CShader::CreateRasterizerState()
{
	
	D3D12_RASTERIZER_DESC d3dRasterizerDesc;
	::ZeroMemory(&d3dRasterizerDesc, sizeof(D3D12_RASTERIZER_DESC));
	d3dRasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	d3dRasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	d3dRasterizerDesc.FrontCounterClockwise = FALSE;
	d3dRasterizerDesc.DepthBias = 0;
	d3dRasterizerDesc.DepthBiasClamp = 0.0f;
	d3dRasterizerDesc.SlopeScaledDepthBias = 0.0f;
	d3dRasterizerDesc.DepthClipEnable = TRUE;
	d3dRasterizerDesc.MultisampleEnable = FALSE;
	d3dRasterizerDesc.AntialiasedLineEnable = FALSE;
	d3dRasterizerDesc.ForcedSampleCount = 0;
	d3dRasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	return(d3dRasterizerDesc);
}

D3D12_DEPTH_STENCIL_DESC CShader::CreateDepthStencilState()
{
	D3D12_DEPTH_STENCIL_DESC d3dDepthStencilDesc;
	::ZeroMemory(&d3dDepthStencilDesc, sizeof(D3D12_DEPTH_STENCIL_DESC));
	d3dDepthStencilDesc.DepthEnable = TRUE;
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

	return(d3dDepthStencilDesc);
}

D3D12_BLEND_DESC CShader::CreateBlendState()
{
	D3D12_BLEND_DESC d3dBlendDesc;
	::ZeroMemory(&d3dBlendDesc, sizeof(D3D12_BLEND_DESC));
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

	return(d3dBlendDesc);
}

void CShader::CreateShader(ID3D12Device *pd3dDevice, ID3D12RootSignature *pd3dGraphicsRootSignature)
{
	ID3DBlob *pd3dVertexShaderBlob = NULL, *pd3dPixelShaderBlob = NULL;


	//�Ʒ��� ���ǵǾ��ִ� Create...�Լ����� ��� �������̵��� ������ ��ü���̴�
	//�׷��� ������ CShader�� ��ӹ��� �ڽ� Ŭ�������� ��� �ڽ��� ������ ������ �����ϴ�.
	D3D12_GRAPHICS_PIPELINE_STATE_DESC d3dPipelineStateDesc;	//�� ����ü�� �׷��Ƚ� ������������ ������� �����Ұ������� �����ϴ� ����ü�̴�. 
	::ZeroMemory(&d3dPipelineStateDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	d3dPipelineStateDesc.pRootSignature = pd3dGraphicsRootSignature;			//��Ʈ �ñ״��Ĵ� ���ҽ����� ����� �����ϰ� ���̴��� ��� �������Ϳ� �Ѱ��ٰ��ΰ��� �����Ѵ�.
	d3dPipelineStateDesc.VS = CreateVertexShader(&pd3dVertexShaderBlob);		//�� �Լ��� ���� �������̵��� �Ǿ������� �������̵��� �������̴��� ȣ��ɰ��̴�.	
	d3dPipelineStateDesc.PS = CreatePixelShader(&pd3dPixelShaderBlob);			//�ȼ����̴��� �������̴��� ����
	d3dPipelineStateDesc.RasterizerState = CreateRasterizerState();				//
	d3dPipelineStateDesc.BlendState = CreateBlendState();						
	d3dPipelineStateDesc.DepthStencilState = CreateDepthStencilState();
	d3dPipelineStateDesc.InputLayout = CreateInputLayout();						//�Է������⸦ ����
	d3dPipelineStateDesc.SampleMask = UINT_MAX;
	d3dPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;		//�������� �����Ҷ� Triangle_list,TriangleStrip������ ��� ���������� ���Ѵ�.
	d3dPipelineStateDesc.NumRenderTargets = 1;				//����Ÿ���� ������ �����Ѵ�. ��ó�� �Ҷ� ����Ÿ���� ������ �ʿ��� ��찡 ��������� ������ ���ʿ� ������
	d3dPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;		//����Ÿ���� �� �ȼ��� ������ ��
	d3dPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;			//���� ������ ����(�� , ���� �� ��)
	d3dPipelineStateDesc.SampleDesc.Count = 1;								//������ �Ű� �� �ʿ� ����
	d3dPipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	HRESULT hResult = pd3dDevice->CreateGraphicsPipelineState(&d3dPipelineStateDesc, __uuidof(ID3D12PipelineState), (void **)&m_pd3dPipelineState);

	if (pd3dVertexShaderBlob) pd3dVertexShaderBlob->Release();
	if (pd3dPixelShaderBlob) pd3dPixelShaderBlob->Release();

	if (d3dPipelineStateDesc.InputLayout.pInputElementDescs) delete[] d3dPipelineStateDesc.InputLayout.pInputElementDescs;
}

void CShader::CreateCbvAndSrvDescriptorHeaps(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, int nConstantBufferViews, int nShaderResourceViews)
{
	//��ũ���� ���̶� ������ �����ϴ°� �����ϱ� ���Ұ��̴�.
	// ���� �����ڸ�, ���ҽ��� �����Ϸ��� ��ũ����(������)�� �ʿ��ϴ�.
	// �� �����ڵ��� �� ���� �־���� �ش� ��ü�� �׸��� �� ���� �����ڵ��� �̿��ؼ� ���ҽ��� ������.
	// �� ���� CBV , SRV �� �� �迭�� ���Ҹ��� �Ҵ��Ѵ�.
	// CBV�� ConstantBufferView ���ڷν� ���̴����� ���� ������ ���� �������̴�.
	// SRV�� ShaderResourceView�� ���ڷν� �ַ� �ؽ�ó���� ���̴��� �ѱ涧 ���ȴ�.
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	d3dDescriptorHeapDesc.NumDescriptors = nConstantBufferViews + nShaderResourceViews; //CBVs + SRVs 
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	d3dDescriptorHeapDesc.NodeMask = 0;

	
	pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void **)&m_pd3dCbvSrvDescriptorHeap);

	
	m_d3dCbvCPUDescriptorStartHandle = m_pd3dCbvSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_d3dCbvGPUDescriptorStartHandle = m_pd3dCbvSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	m_d3dSrvCPUDescriptorStartHandle.ptr = m_d3dCbvCPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * nConstantBufferViews);
	m_d3dSrvGPUDescriptorStartHandle.ptr = m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * nConstantBufferViews);

}

//���� �̺κе� ������ �� �ʿ�� ���� ����
//CBV�� �����ϴ� �Լ�
//�� �Լ��� ȣ��Ǳ� ���� ������ ��ũ���� ���� ����� �� �־���Ѵ�. 
void CShader::CreateConstantBufferViews(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, int nConstantBufferViews, ID3D12Resource *pd3dConstantBuffers, UINT nStride)
{
	D3D12_GPU_VIRTUAL_ADDRESS d3dGpuVirtualAddress = pd3dConstantBuffers->GetGPUVirtualAddress();	//��ũ���� �� ���� �ش� cbv�� ��ġ�ϰ� �ִ� �ּҸ� ��ȯ�Ͽ� ����.
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

//ShaderResourceView�� ����
D3D12_SHADER_RESOURCE_VIEW_DESC GetShaderResourceViewDesc(D3D12_RESOURCE_DESC d3dResourceDesc, UINT nTextureType)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc;
	d3dShaderResourceViewDesc.Format = d3dResourceDesc.Format;		//�䰡 ���ҽ��� ���� ����
	d3dShaderResourceViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;	//�ؽ����� rgba�� �д� ������ �����ش�.
	switch (nTextureType)
	{
	case RESOURCE_TEXTURE2D: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)(d3dResourceDesc.DepthOrArraySize == 1)
	case RESOURCE_TEXTURE2D_ARRAY:
		d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		d3dShaderResourceViewDesc.Texture2D.MipLevels = -1;
		d3dShaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
		d3dShaderResourceViewDesc.Texture2D.PlaneSlice = 0;
		d3dShaderResourceViewDesc.Texture2D.ResourceMinLODClamp = 0.0f;
		break;
	case RESOURCE_TEXTURE2DARRAY: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)(d3dResourceDesc.DepthOrArraySize != 1)
		d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
		d3dShaderResourceViewDesc.Texture2DArray.MipLevels = -1;
		d3dShaderResourceViewDesc.Texture2DArray.MostDetailedMip = 0;
		d3dShaderResourceViewDesc.Texture2DArray.PlaneSlice = 0;
		d3dShaderResourceViewDesc.Texture2DArray.ResourceMinLODClamp = 0.0f;
		d3dShaderResourceViewDesc.Texture2DArray.FirstArraySlice = 0;
		d3dShaderResourceViewDesc.Texture2DArray.ArraySize = d3dResourceDesc.DepthOrArraySize;
		break;
	case RESOURCE_TEXTURE_CUBE: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)(d3dResourceDesc.DepthOrArraySize == 6)
		d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
		d3dShaderResourceViewDesc.TextureCube.MipLevels = -1;
		d3dShaderResourceViewDesc.TextureCube.MostDetailedMip = 0;
		d3dShaderResourceViewDesc.TextureCube.ResourceMinLODClamp = 0.0f;
		break;
	case RESOURCE_BUFFER: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
		d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		d3dShaderResourceViewDesc.Buffer.FirstElement = 0;
		d3dShaderResourceViewDesc.Buffer.NumElements = 0;
		d3dShaderResourceViewDesc.Buffer.StructureByteStride = 0;
		d3dShaderResourceViewDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
		break;
	}
	return(d3dShaderResourceViewDesc);
}

void CShader::CreateShaderResourceViews(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CTexture *pTexture, UINT nRootParameterStartIndex, bool bAutoIncrement)
{
	/*D3D12_CPU_DESCRIPTOR_HANDLE d3dSrvCPUDescriptorHandle = m_d3dSrvCPUDescriptorStartHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE d3dSrvGPUDescriptorHandle = m_d3dSrvGPUDescriptorStartHandle;*/
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
		pTexture->SetRootArgument(i, (bAutoIncrement) ? (nRootParameterStartIndex + i) : nRootParameterStartIndex, m_d3dSrvGPUDescriptorStartHandle);
		m_d3dSrvGPUDescriptorStartHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
	}
}

void CShader::CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
}

void CShader::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
}

void CShader::UpdateShaderVariable(ID3D12GraphicsCommandList *pd3dCommandList, XMFLOAT4X4 *pxmf4x4World)
{
}

void CShader::ReleaseShaderVariables()
{
	if (m_pd3dCbvSrvDescriptorHeap) m_pd3dCbvSrvDescriptorHeap->Release();
}

void CShader::ReleaseUploadBuffers()
{
}

void CShader::OnPrepareRender(ID3D12GraphicsCommandList *pd3dCommandList)
{
	// ���� ���� SetGraphicsRootSignature ������ �ʿ��ϴ� .
	// ������ CScene::Render�Լ� �� ó���� �̹� ������ �������Ƿ� �ٽ� set���� �ʿ�� ����.
	// ������ ���� �����ϴ� ���̴��� �ƴ϶��(�� , ��ó�� ���̴����) 
	// ���� SetGraphicsRootSignature������ ����� �Ұ��̴�.
	// ���� ���� 1.
	//pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);

	
	// ���� ���� 2. 
	// CreateShader �Լ����� ������ ���������� ������Ʈ�� 
	// SetPipelineState()�� set���ش�. (�� ������ ���ؼ� VS,PS���� ���̴�����)

	if (m_pd3dPipelineState) pd3dCommandList->SetPipelineState(m_pd3dPipelineState);	//�׸��� ����
	
	// ���� ���� 3.
	// ��Ʈ����� �ƴ� cbv,srv, ���̺� �ִ� �����ڵ��� 
	// SetDescriptorHeaps()�� set�� ����� �Ѵ�. 
	pd3dCommandList->SetDescriptorHeaps(1, &m_pd3dCbvSrvDescriptorHeap);				//camera


	// �� �Լ��� CShader�� ����� �ڽ� Ŭ�������� ���� �� �Լ��� �������̵� ������
	// �������̵��� �Լ��� ��� ȣ���
	UpdateShaderVariables(pd3dCommandList);
}

void CShader::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	OnPrepareRender(pd3dCommandList);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CPlayerShader::CPlayerShader()
{
}

CPlayerShader::~CPlayerShader()
{
}

//�Է������� CPlayerShader���� �������̵� ���� 
D3D12_INPUT_LAYOUT_DESC CPlayerShader::CreateInputLayout()
{

	//�Է�������� �� ������ ���鶧 ������ �� �������� ���⼭ ������ش�. 
	// ���� �÷��̾ �����ϴ� �������� ��ġ���� ���������� ���´�.
	// �� �������� ����ǥ��� �����Ǿ� �ֱ� ������  ���̴� ���� ������ ��� ���ۺ�� �Ѱ��� ���� ��ȯ ���, �亯ȯ ���, ������ȯ ��İ� ���Ͽ����� ��ü�� ���� ������ �������� 
	// ���̴� ���� ������ ��ǥ�躯ȯ�� �̷������.  //�� cpu�� �ϴ����� �� ��ǥ���� ������ ���̴��� �Ѱ��ִ°�.
	UINT nInputElementDescs = 2;
	D3D12_INPUT_ELEMENT_DESC *pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	pd3dInputElementDescs[0] ={ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[1] ={ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
	d3dInputLayoutDesc.NumElements = nInputElementDescs;

	return(d3dInputLayoutDesc);
}
D3D12_DEPTH_STENCIL_DESC CPlayerShader::CreateDepthStencilState()
{
	//������ �� �ʿ� ����
	// ���� ���ٽ� ����ü
	D3D12_DEPTH_STENCIL_DESC d3dDepthStencilDesc;
	::ZeroMemory(&d3dDepthStencilDesc, sizeof(D3D12_DEPTH_STENCIL_DESC));
	d3dDepthStencilDesc.DepthEnable = TRUE;								
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

	return(d3dDepthStencilDesc);
}

D3D12_SHADER_BYTECODE CPlayerShader::CreateVertexShader(ID3DBlob **ppd3dShaderBlob)
{
	//Shaders.hlsl���� VSPlayer�Լ��� ���������ش�. �������̵��� VSPlayer�� ������.
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSPlayer", "vs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CPlayerShader::CreatePixelShader(ID3DBlob **ppd3dShaderBlob)
{
	//Shaders.hlsl ���� PSPlayer �Լ��� ������ ���ش�. 
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSPlayer", "ps_5_1", ppd3dShaderBlob));
}

void CPlayerShader::CreateShader(ID3D12Device *pd3dDevice, ID3D12RootSignature *pd3dGraphicsRootSignature)
{
	//�������̵��� �Լ����� ������ �������̵��� �Լ����� GRAPHICS_PIPELINE_STATE�� �������ش�.
	CShader::CreateShader(pd3dDevice, pd3dGraphicsRootSignature);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CTexturedShader::CTexturedShader()
{
}

CTexturedShader::~CTexturedShader()
{
}

D3D12_INPUT_LAYOUT_DESC CTexturedShader::CreateInputLayout()
{
	UINT nInputElementDescs = 2;
	D3D12_INPUT_ELEMENT_DESC *pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];




	pd3dInputElementDescs[0] ={ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[1] ={ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 };

	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
	d3dInputLayoutDesc.NumElements = nInputElementDescs;

	return(d3dInputLayoutDesc);
}

D3D12_SHADER_BYTECODE CTexturedShader::CreateVertexShader(ID3DBlob **ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSTextured", "vs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CTexturedShader::CreatePixelShader(ID3DBlob **ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSTextured", "ps_5_1", ppd3dShaderBlob));
}

void CTexturedShader::CreateShader(ID3D12Device *pd3dDevice, ID3D12RootSignature *pd3dGraphicsRootSignature)
{
	CShader::CreateShader(pd3dDevice, pd3dGraphicsRootSignature);
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CTerrainShader::CTerrainShader()
{
}

CTerrainShader::~CTerrainShader()
{
}

D3D12_INPUT_LAYOUT_DESC CTerrainShader::CreateInputLayout()
{
	UINT nInputElementDescs = 4;
	D3D12_INPUT_ELEMENT_DESC *pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	pd3dInputElementDescs[0] ={ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[1] ={ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[2] ={ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 28, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[3] = { "TEXCOORD" ,1, DXGI_FORMAT_R32G32_FLOAT,0,36,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 };
	


	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
	d3dInputLayoutDesc.NumElements = nInputElementDescs;

	return(d3dInputLayoutDesc);
}

D3D12_SHADER_BYTECODE CTerrainShader::CreateVertexShader(ID3DBlob **ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSTerrain", "vs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CTerrainShader::CreatePixelShader(ID3DBlob **ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSTerrain", "ps_5_1", ppd3dShaderBlob));
}

void CTerrainShader::CreateShader(ID3D12Device *pd3dDevice, ID3D12RootSignature *pd3dGraphicsRootSignature)
{
	CShader::CreateShader(pd3dDevice, pd3dGraphicsRootSignature);
}



///////////////////////////////////////////////////////////////////////////////////////
//billboardShader
CBillboardShader::CBillboardShader()
{

}

CBillboardShader::~CBillboardShader()
{

}

void CBillboardShader::CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(CB_GAMEOBJECT_INFO) + 255) & ~255); //256�� ���

																		//���ҽ��� �������� ���� ���� 
																		//D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER -> ���� ���� �Ǵ� ��� ���۷� ���ɶ� �ʱ� STATE

																		//���ε���
	m_pd3dcbBillboardObjects = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes * m_nObjects, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	//���ҽ��� ���� CPU�����͸� ��ȯ (���� - ������ ����)
	m_pd3dcbBillboardObjects->Map(0, NULL, (void **)&m_pcbMappedBillboardObjects);	//m_pcbMappedGameObjects �� ���������� cpu�� ���� ������ �ּҰ� �ȴ�. �� ���ҽ��� ���� �����ʹ� ���������� ��ġ�� �ȹٲ۴�.
																			//�ֳ��ϸ� cpu�� ���� �����ϴ� ���ҽ��̱� ������
																			//m_pcbMappedGameObjects�� �б⸦ ���� ���ƾ��Ѵ�.(���ɻ� ������ �߻�)

}

void CBillboardShader::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{

	// �� �Լ��� CShader::UpdateShaderVariables �Լ��� �������̵� �Ѱ����ν�
	// CreateShaderVariables���� ������۷� ���� �޸��� �Ҵ��� �س��ٸ�  
	// UpdateShaderVaribles�Լ��� ���� �� �Լ����� �� ��ü�鸶�� �����ִ� ���� ��ǥ���� 

	// Object.h ���������� ����Ǿ��ִ� �Ʒ��� ����ü�� �������ش�. 
	// �� ����ü�� Shaders.hlsl�� cbGameObjectInfo�� ���Ŀ� ���� ���� �ٰ��̴�. 
	//struct CB_GAMEOBJECT_INFO
	//{
	//	XMFLOAT4X4						m_xmf4x4World;
	//	UINT							m_nObjectID;
	//};

	UINT ncbElementBytes = ((sizeof(CB_GAMEOBJECT_INFO) + 255) & ~255);
	for (int j = 0; j < m_nObjects; j++)
	{
		CB_GAMEOBJECT_INFO *pbMappedcbGameObject = (CB_GAMEOBJECT_INFO *)((UINT8 *)m_pcbMappedBillboardObjects + (j * ncbElementBytes));
		XMStoreFloat4x4(&pbMappedcbGameObject->m_xmf4x4World, XMMatrixTranspose(XMLoadFloat4x4(&m_ppObjects[j]->m_xmf4x4World)));

	}

}

void CBillboardShader::ReleaseUploadBuffers()
{
	if (m_ppObjects)
	{
		for (int j = 0; j < m_nObjects; j++) if (m_ppObjects[j]) m_ppObjects[j]->ReleaseUploadBuffers();
	}

#ifdef _WITH_BATCH_MATERIAL
	if (m_pMaterial) m_pMaterial->ReleaseUploadBuffers();
#endif
}

void CBillboardShader::ReleaseShaderVariables()
{
	if (m_pd3dcbBillboardObjects)
	{
		m_pd3dcbBillboardObjects->Unmap(0, NULL);
		m_pd3dcbBillboardObjects->Release();
	}

	CTexturedShader::ReleaseShaderVariables();
}

void CBillboardShader::ReleaseObjects()
{
	if (m_ppObjects)
	{
		for (int j = 0; j < m_nObjects; j++) if (m_ppObjects[j]) delete m_ppObjects[j];
		delete[] m_ppObjects;
	}

#ifdef _WITH_BATCH_MATERIAL
	if (m_pMaterial) delete m_pMaterial;
#endif
}

void CBillboardShader::AnimateObjects(float fTimeElapsed,CCamera *pCamera,CPlayer* pPlayer)
{
	
	for (int j = 0; j < m_nObjects; j++)
	{

		m_ppObjects[j]->Animate(fTimeElapsed,pCamera);
		

	}
}

void CBillboardShader::BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, void *pContext)
{

	//���̴� Ŭ�������� ��ġ ó���ؾ��� ������Ʈ���� �����ϴ� �Լ�

	//������� �������� ��ġ�ؾ��ϱ� ������ ������ ������ �޾ƿ;��Ѵ�. 

	CHeightMapTerrain *pTerrain = (CHeightMapTerrain *)pContext;



	//�� �������� �� ������Ʈ�� �󸶸�ŭ �Ÿ��� �������� �����ɰ��̳ĸ� �����ϴµ� 
	// �̰��� �� �ʿ� ����. ������ ���ۿ��� �̷������� ���� ���̱� ������ 
	// ������ ���ڿ�������  ��Ģ������ �Ǿ��ֱ� ����
	float fxPitch = 100.0f;				//�� ���� Ŀ������ ������Ʈ�� ���� ��������. fxPitch ������ x��ǥ ��ȭ��
	float fyPitch = 1.0f;
	float fzPitch = 100.0f;				//

	//������ ���α��� ���α��̸� �޾ƿ´�. 
	float fTerrainWidth = pTerrain->GetWidth();
	float fTerrainLength = pTerrain->GetLength();


	// �� �������� ���� ��ġ�ؾ��� ������Ʈ ���� �����Ѵ�. 
	int xObjects = int(fTerrainWidth / fxPitch);
	int yObjects = 1;
	int zObjects = int(fTerrainLength / fzPitch);
	//CTexture *pTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	//pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Texture/Tree01s.dds", 0);




	// �� �κ��� dds������ �ε��ϴ� �κ��̴�. 
	// ������ �ڼ��� ���ʿ�� ���� ���δ� ����μ���
	CTexture *ppTextures[TEXTURES];
	ppTextures[0] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	ppTextures[0]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Texture/tree01s.dds", 0);
	ppTextures[1] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	ppTextures[1]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Texture/tree02S.dds", 0);
	ppTextures[2] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	ppTextures[2]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Texture/tree35S.dds", 0);
	ppTextures[3] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	ppTextures[3]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Texture/tree4.dds", 0);



	/*CTexture *pTextureArray = new CTexture(1, RESOURCE_TEXTURE2DARRAY, 0);
	pTextureArray->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Texture/treearray.dds", 0);*/


	m_nObjects = (xObjects * yObjects * zObjects);

	UINT ncbElementBytes = ((sizeof(CB_GAMEOBJECT_INFO) + 255) & ~255);




	//�̺κп��� ������ �߿��ϴ�. 
	// ��ũ���� �� ���� - ��� ���ۺ並 ����Ѵٸ� ���� �޸𸮿� ���ҽ� ���� - �� ����
	// �� �����̴�. 
	CreateCbvAndSrvDescriptorHeaps(pd3dDevice, pd3dCommandList, m_nObjects, TEXTURES);				//m_nObjects��ŭ�� ��� ���ۺ�� 1���� srv
	CreateShaderVariables(pd3dDevice, pd3dCommandList);										//���ε����� ���ҽ� ����
	CreateConstantBufferViews(pd3dDevice, pd3dCommandList, m_nObjects, m_pd3dcbBillboardObjects, ncbElementBytes); //������Ʈ�� ��� ���ۺ� ����
	
	// Srv�� �����ϰ� ��Ʈ �Ķ���Ϳ� �������ش� .
	// ������ bool���ڰ� true, false ���� ������ �ٸ����� �����ϰڴ�. 
	for (int i = 0; i < TEXTURES; i++) {
		CreateShaderResourceViews(pd3dDevice, pd3dCommandList, ppTextures[i], 8, false);				//SRV ���� �� ����
	}
	

	CMaterial *ppMaterials[TEXTURES];
	for (int i = 0; i < TEXTURES; i++)
	{
		ppMaterials[i] = new CMaterial();
		ppMaterials[i]->SetTexture(ppTextures[i]);
	}


	// ������ �޽ø� �����ϰ� SetMesh�Լ��� �̿��� �� ������Ʈ�� �޽��� set���ش�. 
	//CCubeMeshTextured *pCubeMesh = new CCubeMeshTextured(pd3dDevice, pd3dCommandList, 12.0f, 12.0f, 12.0f);
	CTexturedRectMesh* pBillboardMesh = new CTexturedRectMesh(pd3dDevice, pd3dCommandList, 50.0f, 50.0f, 0.0f,0.0f,0.0f,-10.0f );

	m_ppObjects = new CGameObject*[m_nObjects];

	XMFLOAT3 xmf3RotateAxis, xmf3SurfaceNormal;
	CBillboardObject* pBillboardObject = NULL;

	for (int i = 0, x = 0; x < xObjects; x++)
	{
		for (int z = 0; z< zObjects; z++)
		{

				
				//////////////////////////////////////////////////////////////////
				//�� ������ ������ ���⸦ �о�ͼ� 
				//������ �����ϴٰ� �����Ǹ�
				//�����带 ���� ����
				pBillboardObject = new CBillboardObject();
				pBillboardObject->SetMesh(0, pBillboardMesh);
//#ifndef _WITH_BATCH_MATERIAL
//				pBillboardObject->SetMaterial(pCubeMaterial);
//#endif
				pBillboardObject->SetMaterial(ppMaterials[i % TEXTURES]);	//
				float xPosition = x * fxPitch;
				float zPosition = z * fzPitch;
				float fHeight = pTerrain->GetHeight(xPosition, zPosition);		//���⼭ ������ ��ġ�� �޾ƿ´�.
				if (fHeight > 100.0f) {
					pBillboardObject->SetPosition(xPosition, fHeight + 18.0f, zPosition);	//������ ��ġ ����
				}
				//////////////////////////////////////////////////////////
				//����Ʒ��� ������ �����Ƿ� �ǵ帱�ʿ� �����

		
				xmf3SurfaceNormal = pTerrain->GetNormal(xPosition, zPosition);
				xmf3RotateAxis = Vector3::CrossProduct(XMFLOAT3(0.0f, 1.0f, 0.0f), xmf3SurfaceNormal);
				if (Vector3::IsZero(xmf3RotateAxis))
				{
					xmf3RotateAxis = XMFLOAT3(0.0f, 1.0f, 0.0f);
				}

				float fAngle = acos(Vector3::DotProduct(XMFLOAT3(0.0f, 1.0f, 0.0f), xmf3SurfaceNormal));
				pBillboardObject->Rotate(&xmf3RotateAxis, XMConvertToDegrees(fAngle));
			
				//gpu�� �ش� �����͸� �����־�� �ϱ⶧���� �ݵ�� �Ʒ� �Լ��� ȣ���ؾ���. 
				pBillboardObject->SetCbvGPUDescriptorHandlePtr(m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize*i));
				m_ppObjects[i++] = pBillboardObject;
			
			
		}
	}

}

void CBillboardShader::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	//
	CTexturedShader::Render(pd3dCommandList, pCamera);
/////////////////////////////////////////////////////////////////////////////////

	//���� �Լ��� �Ʒ� ������ ����Ȱ��̶� �����ϸ��. 
	// CShader::OnPrepareRender ����

	//pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);
	//if (m_pd3dPipelineState) pd3dCommandList->SetPipelineState(m_pd3dPipelineState);	//�׸��� ����
	//pd3dCommandList->SetDescriptorHeaps(1, &m_pd3dCbvSrvDescriptorHeap);				//camera
	//UpdateShaderVariables(pd3dCommandList);

/////////////////////////////////////////////////////////////////////////////////

	//���� ������ ���� �Ǹ� 
	// ��ġ ó���� �⺻���� �غ� �Ȱ��̶� �����ص��ȴ�.
	// �Ʒ��� for���� �� ������Ʈ �ϳ��ϳ��� �׸��� �����̴�. 
	// Render() �Լ��� ������
	for (int j = 0; j < m_nObjects; j++)
	{
		if (m_ppObjects[j]) m_ppObjects[j]->Render(pd3dCommandList, pCamera,j);
	}
}


/////////////////////////////////////////////////////////////////////////////////////
//CGrassBillboardShader 
//
CGrassBillboardShader::CGrassBillboardShader()
{

}

CGrassBillboardShader::~CGrassBillboardShader()
{}

D3D12_SHADER_BYTECODE CGrassBillboardShader::CreatePixelShader(ID3DBlob **ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSGrassTextured", "ps_5_1", ppd3dShaderBlob));
}

void CGrassBillboardShader::CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(CB_GAMEOBJECT_INFO) + 255) & ~255); //256�� ���

																		//���ҽ��� �������� ���� ���� 
																		//D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER -> ���� ���� �Ǵ� ��� ���۷� ���ɶ� �ʱ� STATE
																		//���ε���
	m_pd3dcbBillboardObjects = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes * m_nObjects, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	//���ҽ��� ���� CPU�����͸� ��ȯ (���� - ������ ����)
	m_pd3dcbBillboardObjects->Map(0, NULL, (void **)&m_pcbMappedBillboardObjects);	//m_pcbMappedGameObjects �� ���������� cpu�� ���� ������ �ּҰ� �ȴ�. �� ���ҽ��� ���� �����ʹ� ���������� ��ġ�� �ȹٲ۴�.
																					//�ֳ��ϸ� cpu�� ���� �����ϴ� ���ҽ��̱� ������

}

void CGrassBillboardShader::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{

	// �� �Լ��� CShader::UpdateShaderVariables �Լ��� �������̵� �Ѱ����ν�
	// CreateShaderVariables���� ������۷� ���� �޸��� �Ҵ��� �س��ٸ�  
	// UpdateShaderVaribles�Լ��� ���� �� �Լ����� �� ��ü�鸶�� �����ִ� ���� ��ǥ���� 

	// Object.h ���������� ����Ǿ��ִ� �Ʒ��� ����ü�� �������ش�. 
	// �� ����ü�� Shaders.hlsl�� cbGameObjectInfo�� ���Ŀ� ���� ���� �ٰ��̴�. 
	//struct CB_GAMEOBJECT_INFO
	//{
	//	XMFLOAT4X4						m_xmf4x4World;
	//	UINT							m_nObjectID;
	//};
	

	UINT ncbElementBytes = ((sizeof(CB_GAMEOBJECT_INFO) + 255) & ~255);

	

	for (int j = 0; j < m_nObjects; j++)
	{

		CB_GAMEOBJECT_INFO *pbMappedcbGameObject = (CB_GAMEOBJECT_INFO *)((UINT8 *)m_pcbMappedBillboardObjects + (j * ncbElementBytes));
		XMStoreFloat4x4(&pbMappedcbGameObject->m_xmf4x4World, XMMatrixTranspose(XMLoadFloat4x4(&m_ppObjects[j]->m_xmf4x4World)));

	}


}


void CGrassBillboardShader::ReleaseShaderVariables()
{
	if (m_pd3dcbBillboardObjects)
	{
		m_pd3dcbBillboardObjects->Unmap(0, NULL);
		m_pd3dcbBillboardObjects->Release();
	}



	CTexturedShader::ReleaseShaderVariables();
}

void CGrassBillboardShader::ReleaseUploadBuffers()
{
	if (m_ppObjects)
	{
		for (int j = 0; j < m_nObjects; j++) if (m_ppObjects[j]) m_ppObjects[j]->ReleaseUploadBuffers();
	}

#ifdef _WITH_BATCH_MATERIAL
	if (m_pMaterial) m_pMaterial->ReleaseUploadBuffers();
#endif

}

void CGrassBillboardShader::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera* pCamera = NULL)
{
	CTexturedShader::Render(pd3dCommandList, pCamera);

#ifdef _WITH_BATCH_MATERIAL
	if (m_pMaterial) m_pMaterial->UpdateShaderVariables(pd3dCommandList);
#endif
	for (int j = 0; j < m_nObjects; j++)
	{
		if (m_ppObjects[j]) m_ppObjects[j]->Render(pd3dCommandList, pCamera,j);
	}
}

void CGrassBillboardShader::ReleaseObjects()
{
	if (m_ppObjects)
	{
		for (int j = 0; j < m_nObjects; j++) if (m_ppObjects[j]) delete m_ppObjects[j];
		delete[] m_ppObjects;
	}

#ifdef _WITH_BATCH_MATERIAL
	if (m_pMaterial) delete m_pMaterial;
#endif
}

void CGrassBillboardShader::BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, void *pContext)
{


	//���̴� Ŭ�������� ��ġ ó���ؾ��� ������Ʈ���� �����ϴ� �Լ�

	//������� �������� ��ġ�ؾ��ϱ� ������ ������ ������ �޾ƿ;��Ѵ�. 
	CHeightMapTerrain *pTerrain = (CHeightMapTerrain *)pContext;


	//�� �������� �� ������Ʈ�� �󸶸�ŭ �Ÿ��� �������� �����ɰ��̳ĸ� �����ϴµ� 
	// �̰��� �� �ʿ� ����. ������ ���ۿ��� �̷������� ���� ���̱� ������ 
	// ������ ���ڿ�������  ��Ģ������ �Ǿ��ֱ� ����
	float fxPitch = 50.0f;				//�� ���� Ŀ������ ������Ʈ�� ���� ��������. fxPitch ������ x��ǥ ��ȭ��
	float fyPitch = 1;
	float fzPitch = 50.0f;				//

	//������ ���α��� ���α��̸� �޾ƿ´�. 
	float fTerrainWidth = pTerrain->GetWidth();
	float fTerrainLength = pTerrain->GetLength();

	// �� �������� ���� ��ġ�ؾ��� ������Ʈ ���� �����Ѵ�. 
	int xObjects = int(fTerrainWidth / fxPitch);
	int yObjects = 1;
	int zObjects = int(fTerrainLength / fzPitch);

	// �� �κ��� dds������ �ε��ϴ� �κ��̴�. 
	// ������ �ڼ��� ���ʿ�� ���� ���δ� ����μ���

	CTexture *ppTexture[3];
	ppTexture[0] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	ppTexture[0]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Texture/grass01.dds", 0);
	ppTexture[1] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	ppTexture[1]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Texture/Grass02.dds", 0);
	ppTexture[2] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	ppTexture[2]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Texture/grass04.dds", 0);


	m_nObjects = (xObjects * yObjects * zObjects);

	UINT ncbElementBytes = ((sizeof(CB_GAMEOBJECT_INFO) + 255) & ~255);
	//�̺κп��� ������ �߿��ϴ�. 
	// ��ũ���� �� ���� - ��� ���ۺ並 ����Ѵٸ� ���� �޸𸮿� ���ҽ� ���� - �� ����
	// �� �����̴�. 

	CreateCbvAndSrvDescriptorHeaps(pd3dDevice, pd3dCommandList, m_nObjects, 3);
	CreateShaderVariables(pd3dDevice, pd3dCommandList);										//���ε����� ���ҽ� ����
	CreateConstantBufferViews(pd3dDevice, pd3dCommandList, m_nObjects, m_pd3dcbBillboardObjects, ncbElementBytes); //������Ʈ�� ��� ���ۺ� ����
	
	
	//7�� �Ķ���Ϳ� ���� �� dds �ؽ��� �������ְ� �ִ°�. (�� , �� �Ķ���͸� ����ϴ°��̴�)
	// �̰ͱ��� �ڼ��� ���ʿ� �����
	for (int i = 0; i < 3; i++) {
		CreateShaderResourceViews(pd3dDevice, pd3dCommandList, ppTexture[i], 7, false);				//SRV ���� �� ����
	}
//#ifdef _WITH_BATCH_MATERIAL
//	m_pMaterial = new CMaterial();
//	m_pMaterial->SetTexture(pTexture);
//#else
//	CMaterial *pCubeMaterial = new CMaterial();
//	pCubeMaterial->SetTexture(pTexture);
//#endif
	CMaterial *ppMaterials[3];
	for (int i = 0; i < 3; i++)
	{
		ppMaterials[i] = new CMaterial();
		ppMaterials[i]->SetTexture(ppTexture[i]);
	}



	CTexturedRectMesh* pBillboardMesh = new CTexturedRectMesh(pd3dDevice, pd3dCommandList, 15.0f, 15.0f, 0.0f, 0.0f, 0.0f, -10.0f);

	m_ppObjects = new CGameObject*[m_nObjects];

	XMFLOAT3 xmf3RotateAxis, xmf3SurfaceNormal;
	CGrassBillboardObject* pBillboardObject = NULL;
	for (int i = 0, x = 0; x < xObjects; x++)
	{
		for (int z = 0; z< zObjects; z++)
		{
			for (int y = 0; y < yObjects; y++) {
				pBillboardObject = new CGrassBillboardObject();
				pBillboardObject->SetMesh(0, pBillboardMesh);
//#ifndef _WITH_BATCH_MATERIAL
//				pBillboardObject->SetMaterial(pCubeMaterial);
//#endif

				//pBillboardObject->SetOOBB(XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(2.0f, 1.0f, 2.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
				pBillboardObject->SetMaterial(ppMaterials[i % 3]);
				float xPosition = x * fxPitch + (rand()%10);
				float zPosition = z * fzPitch + (rand()%11); 
				float fHeight = pTerrain->GetHeight(xPosition, zPosition);

				pBillboardObject->SetPosition(xPosition, fHeight + (y * 3.0f * fyPitch)+8.0f , zPosition);
				pBillboardObject->SetOOBB(XMFLOAT3(xPosition, fHeight, zPosition), XMFLOAT3(2.0f, 1.0f, 2.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
				//std::cout << pBillboardObject->GetPosition().x << pBillboardObject->GetPosition().y << pBillboardObject->GetPosition().z << std::endl;
				if (y == 0) {
					xmf3SurfaceNormal = pTerrain->GetNormal(xPosition, zPosition);
					xmf3RotateAxis = Vector3::CrossProduct(XMFLOAT3(0.0f, 1.0f, 0.0f), xmf3SurfaceNormal);
					if (Vector3::IsZero(xmf3RotateAxis))
					{
						xmf3RotateAxis = XMFLOAT3(0.0f, 1.0f, 0.0f);
					}

					float fAngle = acos(Vector3::DotProduct(XMFLOAT3(0.0f, 1.0f, 0.0f), xmf3SurfaceNormal));
					pBillboardObject->Rotate(&xmf3RotateAxis, XMConvertToDegrees(fAngle));
				}

				pBillboardObject->SetCbvGPUDescriptorHandlePtr(m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize*i));
				m_ppObjects[i++] = pBillboardObject;
			}
		}


	}
}

void CGrassBillboardShader::AnimateObjects(float fTimeElapsed, CCamera *pCamera,CPlayer *pPlayer)
{

	// �� ContaintmentType ENUM�����ν� �浹 �˻翡 ���� �ڷ����� ���Ѵ�. 
	//  DISJOINT = 0,INTERSECTS = 1, CONTAINS = 2, �� �ְ� 
	// DISJOINT �� �ƿ� ��ġ�� ���� ���·� �浹���� ���� �����̴�.
	// INTERSECTS �� �� �浹 �� ���¸� �ǹ� �ﰢ������ ������ �׵θ��� ���� ���¶� �� �� �ְڴ�.
	// CONTAINS �� ���� ���ο� ��ġ�� ���¸� �ǹ��Ѵ�. 
	ContainmentType containType = DISJOINT;
	for (int j = 0; j < m_nObjects; j++)
	{
		//Bounding Box�� Boundind Oriented Box ���� �浹 ��ü���� 
		// Contains�Լ��� Intersects �Լ��� ������ �浹�˻縦 �����ϰ� �����ش�. 
		containType=pPlayer->m_sphereBox->Contains(m_ppObjects[j]->m_xmOOBB);

		// �Ʒ� �ڵ�� Ǯ���� �÷��̾��� ��ü(���� ������ ������ �÷��̾ �߽����� ������ 100m�� Bounding Box�� �ִ�)
		// CPlayer������ ���� �ٶ�. 
		// 
		if (containType == CONTAINS)
		{
			m_ppObjects[j]->Animate(fTimeElapsed, pCamera, true);	//true�ϰ�� �÷��̾� �ݰ� 100m�� ���°�
		}
		else {
			m_ppObjects[j]->Animate(fTimeElapsed, pCamera, false);	//false �ϰ�� �÷��̾� �ݰ� 100m �ۿ� �ִ°�
		}
	
	}
}


//////////////////////////////////////////////////////////////////
CSkyBoxShader::CSkyBoxShader()
{

}

CSkyBoxShader::~CSkyBoxShader()
{

}

D3D12_DEPTH_STENCIL_DESC CSkyBoxShader::CreateDepthStencilState()
{
	D3D12_DEPTH_STENCIL_DESC d3dDepthStencilDesc;
	d3dDepthStencilDesc.DepthEnable = FALSE;
	d3dDepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	d3dDepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_NEVER;
	d3dDepthStencilDesc.StencilEnable = FALSE;
	d3dDepthStencilDesc.StencilReadMask = 0xff;
	d3dDepthStencilDesc.StencilWriteMask = 0xff;
	d3dDepthStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_INCR;
	d3dDepthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	d3dDepthStencilDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_DECR;
	d3dDepthStencilDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;

	return(d3dDepthStencilDesc);
}

D3D12_SHADER_BYTECODE CSkyBoxShader::CreatePixelShader(ID3DBlob **ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSSkyBox", "ps_5_1", ppd3dShaderBlob));
}

void CSkyBoxShader::CreateShader(ID3D12Device *pd3dDevice, ID3D12RootSignature *pd3dGraphicsRootSignature)
{
	CShader::CreateShader(pd3dDevice, pd3dGraphicsRootSignature);
}
