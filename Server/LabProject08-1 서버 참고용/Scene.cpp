//-----------------------------------------------------------------------------
// File: CScene.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Scene.h"
#include "Player.h"



CScene::CScene()
{
}

CScene::~CScene()
{
}

//#define _WITH_TERRAIN_PARTITION

void CScene::BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	//��Ʈ �ñ״��ĸ� �����ϰ� CScene������ ���� ���ҽ����� �����ϰ� �������� ���� 
	//�� , Scene->BuildObjects���� �����ϰ� �ִ� ��ü�鿡�� �ʿ��� ���ҽ����� CScene�� ��Ʈ�ñ״��Ŀ��� ���̴����� �������ִ°��̴�. 
	m_pd3dGraphicsRootSignature = CreateGraphicsRootSignature(pd3dDevice);			//��Ʈ �ñ׳��� ���� 

	XMFLOAT3 xmf3Scale(8.0f, 2.0f, 8.0f);
	XMFLOAT4 xmf4Color(0.0f, 0.5f, 0.0f, 0.0f);
	// ��ī�� �ڽ� 
	m_pSkyBox = new CSkyBox(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);

#ifdef _WITH_TERRAIN_PARTITION
	m_pTerrain = new CHeightMapTerrain(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, _T("Image/ImageHeightMap.raw"), 257, 257, 17, 17, xmf3Scale, xmf4Color);
//	m_pTerrain = new CHeightMapTerrain(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, _T("../Assets/Image/Terrain/HeightMap.raw"), 257, 257, 17, 17, xmf3Scale, xmf4Color);
#else
	// CHeightMapTerrain�� ���������� ����
	m_pTerrain = new CHeightMapTerrain(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, _T("Image/HeightMap.raw"), 257, 257, 257, 257, xmf3Scale, xmf4Color,0);
	// CPlaneTerrain�� ������ ����
	m_pPlaneTerrain = new CHeightMapTerrain(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, _T("Image/HeightMapSand2.raw"), 257, 257, 257, 257, xmf3Scale, xmf4Color,1);
//	m_pTerrain = new CHeightMapTerrain(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, _T("../Assets/Image/Terrain/HeightMap.raw"), 257, 257, 257, 257, xmf3Scale, xmf4Color);
#endif
	
	// ���� ���̴��� ����Ͽ� �׷����� ��ü���� �ѹ��� set�ϰ� �ѹ��� draw�ϰ� �ϱ� ���Ͽ� , �� ��ġ ó���� ���Ͽ�
	// ShaderŬ������ ������ ������ �����ϰ� �ִ�. 
	// �� �� Ŭ�������� Scene->Render�� ȣ�� �Ǹ� �ش� ���̴����� 
	// m_ppShaders -> Render�� ȣ���Ͽ� ��ġó���Ѵ�. 
	// ��ġ ó���� �ϴ� ������ ������ �������� fps�� ���� �Դ� �����߿� �ϳ��� set And Draw �̴�. 
	// ��ü �ϳ� �׸����� Set�ϰ� Draw�ϰ� �� �ϳ� �׸����� SEt�ϰ� Draw�ϴ� ������ �ݺ��Ǵ°��� �ſ� ��ȿ����.
	// �׷��� ������ ���� ���̴��� ���� ���������� ������ ��ġ�� ������Ʈ���� ��� �ѹ��� set�ϰ� ��� ������Ʈ���� draw�ϴ� ����� ���Ѵ�.
	m_nShaders = 2;
	m_ppShaders = new CShader*[m_nShaders];


	// ������ ���̴��� �����Ҷ� �Ʒ� ������ �ٲ��� �ʴ´�.
	// �� ���̴��� � �������̴�,�ȼ����̴����� ����Ұ������� 
	// CreateShader���� �˷��ְ�
	// �� ���̴� �� �̿��ؼ� �׸� ������Ʈ���� BuildObject���� �������ش�.


	//���� ������ ������Ʈ�� �׸��� ����ϴ� 
	CBillboardShader *pBillboardShader = new CBillboardShader();
	pBillboardShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);
	pBillboardShader->BuildObjects(pd3dDevice, pd3dCommandList, m_pTerrain);
	m_ppShaders[0] = pBillboardShader;

	//Ǯ ������Ʈ�� �׸��� ����� ���̴� Ŭ����
	CGrassBillboardShader *pGrassBillboardShader = new CGrassBillboardShader();
	pGrassBillboardShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);
	pGrassBillboardShader->BuildObjects(pd3dDevice, pd3dCommandList, m_pPlaneTerrain);
	m_ppShaders[1] = pGrassBillboardShader;


	//�� �Լ��� �� �� �������� �Ѱ��ִ� ������۷δ� (����, ���� , �Ȱ�, ����) ������ �������̴�. �ش� ���������� ���� �Ѱ����� ����
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void CScene::ReleaseObjects()
{
	// ���� �����ϰ� �ִ� ��Ʈ �ñ״��� ���� // Com��ü���� �ҷ����� �������̽��̹Ƿ� delete �ϸ� �ȵȴ�. ���� ī��Ʈ�� -1���ش�. 
	if (m_pd3dGraphicsRootSignature) m_pd3dGraphicsRootSignature->Release();

	//���� �����ϰ� �ִ� ���̴� Ŭ������ ����
	if (m_ppShaders)
	{
		for (int i = 0; i < m_nShaders; i++)
		{
			m_ppShaders[i]->ReleaseShaderVariables();
			m_ppShaders[i]->ReleaseObjects();
			m_ppShaders[i]->Release();
		}
		delete[] m_ppShaders;
	}

	ReleaseShaderVariables();

	//���� �����ϴ� ��, ���̴� Ŭ���� ���� ������Ʈ ��ü���� ���̴� ���� �ϰ� �ִ� ��ü���� delete
	if (m_pTerrain) delete m_pTerrain;
}

void CScene::ReleaseUploadBuffers()
{
	for (int i = 0; i < m_nShaders; i++) m_ppShaders[i]->ReleaseUploadBuffers();
	if (m_pTerrain) m_pTerrain->ReleaseUploadBuffers();
}

ID3D12RootSignature *CScene::CreateGraphicsRootSignature(ID3D12Device *pd3dDevice)
{
	//���� �����ϴ� ��Ʈ �ñ״��� 
	// �� pScene->Render�Լ� ���� ���� ���� SetGraphicsRootSignature�� ȣ��Ǿ�� �Ѵ�.
	ID3D12RootSignature *pd3dGraphicsRootSignature = NULL;

	//D3D12_DESCRIPTOR_RANGE�� ������ DescriptorTable�� �� Descriptor���� �����Ѱ��̶� �� �� �ִ�.
	D3D12_DESCRIPTOR_RANGE pd3dDescriptorRanges[9];

	// DescriptorTable�� ù��° ���Ҵ� CBV (��� ���� ��) 
	pd3dDescriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	pd3dDescriptorRanges[0].NumDescriptors = 1;
	pd3dDescriptorRanges[0].BaseShaderRegister = 2; //GameObject
	pd3dDescriptorRanges[0].RegisterSpace = 0;
	pd3dDescriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	//�ι�° ���Ҵ� SRV�� (���̴� ���ҽ� ��)
	pd3dDescriptorRanges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[1].NumDescriptors = 1;
	pd3dDescriptorRanges[1].BaseShaderRegister = 0; //t0: gtxtTexture
	pd3dDescriptorRanges[1].RegisterSpace = 0;
	pd3dDescriptorRanges[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;						//���̽� �ؽ���
	pd3dDescriptorRanges[2].NumDescriptors = 1;
	pd3dDescriptorRanges[2].BaseShaderRegister = 1; //t1: gtxtTerrainBaseTexture
	pd3dDescriptorRanges[2].RegisterSpace = 0;
	pd3dDescriptorRanges[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[3].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;						//������ �ؽ���
	pd3dDescriptorRanges[3].NumDescriptors = 1;
	pd3dDescriptorRanges[3].BaseShaderRegister = 2; //t2: gtxtTerrainDetailTexture
	pd3dDescriptorRanges[3].RegisterSpace = 0;
	pd3dDescriptorRanges[3].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[4].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[4].NumDescriptors = 1;
	pd3dDescriptorRanges[4].BaseShaderRegister = 4;  //t4 : gtxtGrassBillboardTexture
	pd3dDescriptorRanges[4].RegisterSpace = 0;
	pd3dDescriptorRanges[4].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[5].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[5].NumDescriptors = 1;
	pd3dDescriptorRanges[5].BaseShaderRegister = 5;	  //t5 : gtxtSandBaseTexture;
	pd3dDescriptorRanges[5].RegisterSpace = 0;
	pd3dDescriptorRanges[5].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[6].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;						//������ �ؽ���
	pd3dDescriptorRanges[6].NumDescriptors = 1;
	pd3dDescriptorRanges[6].BaseShaderRegister = 3; //t3: gtxtBillboardTexture
	pd3dDescriptorRanges[6].RegisterSpace = 0;
	pd3dDescriptorRanges[6].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[7].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;						//��ī�̹ڽ� �ؽ���
	pd3dDescriptorRanges[7].NumDescriptors = 1;
	pd3dDescriptorRanges[7].BaseShaderRegister = 6; //t6: gtxtSkyBoxTexture
	pd3dDescriptorRanges[7].RegisterSpace = 0;
	pd3dDescriptorRanges[7].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	
	pd3dDescriptorRanges[8].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;						//��ī�̹ڽ� �ؽ���
	pd3dDescriptorRanges[8].NumDescriptors = 1;
	pd3dDescriptorRanges[8].BaseShaderRegister = 8; //t8: gtxtFlowerBillboardTexture
	pd3dDescriptorRanges[8].RegisterSpace = 0;
	pd3dDescriptorRanges[8].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;


	D3D12_ROOT_PARAMETER pd3dRootParameters[12];

	//������

	// �Ķ� ���� Ÿ������ 3������ �ִ�. 
	// 1.D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE 
	// 2.D3D12_ROOT_PARAMETER_TYPE_CBV (D3D12_ROOT_PARAMETER_TYPE_SRV) (D3D12_ROOT_PARAMETER_TYPE_UAV)
	// 3.D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANT

	// 1�� ������ ��ũ���� ���̺�ν� ���̺��� ���� DiscriptorRange�� ���ǵ� ���̺� ��ȣ�� �����ڸ� �������ش�.(���� �����ϸ� CBV�� ���ҽ��� ����Ű�� �������̰� �� �����͸� ����ִ� �迭�̶� �����ϸ� ������) 
	// 2 �� ������ ���ҽ��� ����Ű�� �����ڷν� ���ҽ��� ����Ű�� �����Ͷ�� �����ص� ������ ����.
	// 3�� ������ ��Ʈ ������ �ϴµ� �����Ͱ� �ƴ� ���� �� ���� �������ִ°��̶�� �����ص� ����. (���� 3D 1 ���������� �� ����� ��Ʈ����θ� ��������)
	// 3���� ���� �������� ��Ʈ �ñ״��Ĵ� 32BIT *64 ���� �Ѱ��ε� ��Ʈ ��� �Ѱ��� 32BIT �̰� ����� �Ѱ��شٰ� �ϸ� 32BIT * 16 �� �̹Ƿ� ��Ʈ�ñ״��Ŀ��� �����ϴ� �κ��� ũ�⶧����
	// ���� ���ӿ����� ����� �� ����. 
	// 2�� ��Ʈ �����ڴ� 64BIT�� MAX : 32�� ���� ��Ʈ �ñ״��Ŀ� ���� �����ϴ�
	// 1�� ��Ʈ ��ũ���� ���̺��� 32BIT�� 64������ ���� �����ϴ�. ���� �ؽ��İ� ������ �ʿ��Ұ�� ��ũ���� ���̺��� ����ϴ� ���� ����.
	// ������ �ؽ��Ĵ� ���̺�θ� ���� �����ϴ�. 

	pd3dRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[0].Descriptor.ShaderRegister = 0; //Player
	pd3dRootParameters[0].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	pd3dRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[1].Descriptor.ShaderRegister = 1; //Camera
	pd3dRootParameters[1].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;


	//������ ���̺�

	pd3dRootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;			//���̺��� ù��° ���Ҵ� cbv
	pd3dRootParameters[2].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[2].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[0];
	pd3dRootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;			//t0: gtxtTexture
	pd3dRootParameters[3].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[3].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[1];
	pd3dRootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;						//�ؽ��Ĵ� �Ϲ������� �ȼ����̴����� �̿��

	pd3dRootParameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;				//t1: gtxtTerrainBaseTexture
	pd3dRootParameters[4].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[4].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[2];
	pd3dRootParameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;				//t2: gtxtTerrainDetailTexture
	pd3dRootParameters[5].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[5].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[3];
	pd3dRootParameters[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[6].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;				//t3: gtxtSandboardTexture
	pd3dRootParameters[6].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[6].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[5];
	pd3dRootParameters[6].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[7].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;				//t4 : gxtxGrassBillboardTexture
	pd3dRootParameters[7].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[7].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[4];			
	pd3dRootParameters[7].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[8].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;				//t5 : gxtxBillboardBaseTexture
	pd3dRootParameters[8].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[8].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[6];
	pd3dRootParameters[8].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[9].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;				//t6 : gxtxSkyBoxTexture
	pd3dRootParameters[9].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[9].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[7];
	pd3dRootParameters[9].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[10].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	pd3dRootParameters[10].Descriptor.ShaderRegister = 7;											//t7 : gGameObjectInfos
	pd3dRootParameters[10].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[10].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	pd3dRootParameters[11].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;				//t8 : gxtxFlowerBillboardTexture
	pd3dRootParameters[11].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[11].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[8];
	pd3dRootParameters[11].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;





	//static sampler�ϸ� ���� �� �ϳ� -> ordercolor�� ������� ���� ����
	// ���÷� - ������ �� �ʿ� ������

	D3D12_STATIC_SAMPLER_DESC d3dSamplerDesc;									//���� ���÷� 
	::ZeroMemory(&d3dSamplerDesc, sizeof(D3D12_STATIC_SAMPLER_DESC));
	d3dSamplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;			//���ø��Ҷ� ����� ���͸� ��� (���� ���͸�)
	d3dSamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;			//0~1�� ��� u-��ǥ�� �ذ� ���.
	d3dSamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;			//�ݺ�
	d3dSamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	d3dSamplerDesc.MipLODBias = 0;										//Direct3D�� ����� LOC(�Ӹ�) ������ �� ���� ���� �Ӹ� �������� ���ø�.
	d3dSamplerDesc.MaxAnisotropy = 1;									//���Ͱ�_ �����϶� ���Ǵ� ������(1~16)
	d3dSamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;		//���ø��� ���� ���ϱ� ���� �Լ�
	d3dSamplerDesc.MinLOD = 0;											//�ּ� �Ӹ� ����, �Ӹ� ���� 0�� ���� ũ�� ������ �Ӹ�
	d3dSamplerDesc.MaxLOD = D3D12_FLOAT32_MAX;							//�ִ� �Ӹ� ����, MinLOD���� ũ�ų� ���ƾ���.
	d3dSamplerDesc.ShaderRegister = 0;									//���̴� ��ȣ (s0)
	d3dSamplerDesc.RegisterSpace = 0;
	d3dSamplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;


	
	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
	::ZeroMemory(&d3dRootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	d3dRootSignatureDesc.NumParameters = _countof(pd3dRootParameters);
	d3dRootSignatureDesc.pParameters = pd3dRootParameters;				//��Ʈ �Ķ���͸�����
	d3dRootSignatureDesc.NumStaticSamplers = 1;							//���� ���÷� ���� 1��
	d3dRootSignatureDesc.pStaticSamplers = &d3dSamplerDesc;				//���� ���÷��� ���� ����
	d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;

	ID3DBlob *pd3dSignatureBlob = NULL;
	ID3DBlob *pd3dErrorBlob = NULL;
	D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pd3dSignatureBlob, &pd3dErrorBlob);			//ID3D12Device::CreateRootSignature�� ���޵ɼ� �ִ� ��Ʈ�ñ׳��� ���� 1.0�� ����ȭ��Ų��.
																																	//ù ��° ����:D3D12_ROOT_SIGNATURE_DESC ����ü�� �����ͷν� ��Ʈ �ñ״�ó�� �����Ѵ�.
																																	//��	 ��° ���� : ����ȭ�� ��Ʈ�ñ׳����� �����Ҽ� �ְ� ���ִ� ID3DBlob �������̽��� ����Ű�� �����͸� �޴� �޸� ���
	// �� �κп��� ��Ʈ �ñ״��� ������
	pd3dDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(), pd3dSignatureBlob->GetBufferSize(), __uuidof(ID3D12RootSignature), (void **)&pd3dGraphicsRootSignature);
	//�ι�° ���� : ����ȭ�� ��Ʈ �ñ׳��Ŀ� ���� ������ ������
	//����° ���� : ��Ʈ �ñ׳����� ũ��(����Ʈ ��)
	if (pd3dSignatureBlob) pd3dSignatureBlob->Release();
	if (pd3dErrorBlob) pd3dErrorBlob->Release();

	return(pd3dGraphicsRootSignature);
}

void CScene::CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
}

void CScene::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
}

void CScene::ReleaseShaderVariables()
{
}

bool CScene::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	return(false);
}

bool CScene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	return(false);
}

bool CScene::ProcessInput(UCHAR *pKeysBuffer)
{
	return(false);
}

void CScene::AnimateObjects(float fTimeElapsed)
{
	

	// ���� �����ϴ� ���̴� Ŭ�����鿡�� ���̴� Ŭ������ �����ϴ� ������Ʈ����
	// �����ϰ� ��.
	// ��, cpu���� ó���ؾ��� ������ ������Ʈ�� �̵�, ȸ��, �浹 �˻�, ���� �ǽ���
	for (int i = 0; i < m_nShaders; i++)
	{
		m_ppShaders[i]->AnimateObjects(fTimeElapsed, m_pPlayer->GetCamera(), m_pPlayer);
	}
	
}

void CScene::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{

	// ���� �������� ������ �߿��ϴ� . 
	// SetGraphicsRootSignature�� ȣ�������ν� ������ ����� ���ҽ����� �����
	// ��Ʈ �ñ״��ĸ� set���ش�.
	pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);

	//����Ʈ ������ ������Ʈ�� ��ǻ�� ȭ����� �׷��� ��ġ�� �����ִ°ǵ�
	// ������ �� �ʿ�� �����.
	pCamera->SetViewportsAndScissorRects(pd3dCommandList);
	
	//���� ���� �ִ� Camera ��ü�� �̿��� �� ī�޶�� ������ ����İ� �������� �����
	// Shaders.hlsl ������� 1��(b1)�� �Ѱ��ش�.
	pCamera->UpdateShaderVariables(pd3dCommandList);

	UpdateShaderVariables(pd3dCommandList);

	/////////////////////////////////////////////////////////////////////////////
	// �� �� ��ü���� ��ü �ڽŵ��� ���̴� Ŭ������ ���� �����͸� ���� �����ֱ� ������
	// ��ü�� ���� ȣ���ؼ� �������ش�. 
	// �� ������ �� ��ü���� ���� �� �Ѱ����� �����ϱ⶧���� ��ġó���� �ʿ���� ����
	if (m_pSkyBox) m_pSkyBox->Render(pd3dCommandList, pCamera);
	if (m_pTerrain) m_pTerrain->Render(pd3dCommandList, pCamera);
	if(m_pPlaneTerrain) m_pPlaneTerrain->Render(pd3dCommandList,pCamera);
//////////////////////////////////////////////////////////////////////////////////
	

	// ��ġó���� �ʿ��� ��ü���� �̷������� ���̴� Ŭ������ ���εξ� 
	// �Ѳ����� �׸��� �Ѵ�. (��, set And Draw�� �� ���̴� Ŭ������ �ѹ��� �ǽ���)
	// (���� ������ �ν��Ͻ��̶��� �� �ٸ� �����̹Ƿ� �򰥸��� �ȵ�)
	// ������ �ڼ��� �� �ʿ�� ������ ������� ��ġ ó�������� ������ �˾Ƶθ� ������
	
	
	
	// ���̷�Ʈ x12�� ������ ������������ �����ϱ⿡ �ռ� cpu���� �غ��ؾ��� ������ �ִ�.
	// CShader::OnPrepareRender()�Լ��� �� ������ �ϴ°��̶� �����ִµ�
	// �� �Լ��� ���� �׸��⿡ �ռ� �ؾ� �� �۾����� ���ִ� �Լ���� �����Ҽ� �ְٴ�.
	// ���Լ��� direct �Լ��� �ƴϹǷ� �򰥸��� ����. 
	// �� �̷��� �Լ��� �ִٴ� ���� �� �� ������ ��� CShader�� �ڽ� Ŭ������ 
	// �ڽ��� Render���� �̷��� ������ �ʿ��ϴٴ� �ǹ��̴�. 
	// CShader::OnPrepareRender() �Լ��� ������ 
	
	for (int i = 0; i < m_nShaders; i++)
	{
		m_ppShaders[i]->Render(pd3dCommandList, pCamera);
	}
}

