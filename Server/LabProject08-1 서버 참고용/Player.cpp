//-----------------------------------------------------------------------------
// File: CPlayer.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Player.h"
#include "Shader.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CPlayer

CPlayer::CPlayer(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, void *pContext, int nMeshes) : CGameObject(nMeshes)
{
	m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
	m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);

	m_xmf3Velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_xmf3Gravity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_fMaxVelocityXZ = 0.0f;
	m_fMaxVelocityY = 0.0f;
	m_fFriction = 0.0f;

	m_fPitch = 0.0f;
	m_fRoll = 0.0f;
	m_fYaw = 0.0f;

	// �̺κ��� ���� ������ ������ ������ 100m �� �浹��ü�� �÷��̾ �߽����� �̵��Ѵ�.

	m_sphereBox = new BoundingSphere(XMFLOAT3(0.0f, 0.0f, 0.0f), 100.0f);
}

CPlayer::~CPlayer()
{
	ReleaseShaderVariables();

	if (m_pCamera) delete m_pCamera;
}

void CPlayer::CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	//�÷��̾��� CreateShaderVariables ���� �ڽ��� ī�޶��� �������(���̴� ���� ���� ����)�� �����ϰ� �ʱ�ȭ ���ִ� �κ��� ����
	if (m_pCamera) m_pCamera->CreateShaderVariables(pd3dDevice, pd3dCommandList);

	//ncbElementBytes �� �� ������� ����ü�� ũ�⸦ ��� ����
	UINT ncbElementBytes = ((sizeof(CB_PLAYER_INFO) + 255) & ~255); //256�� ���
	// �� ũ�� ��ŭ�� ������ ���� �޸��� �����ϰ�
	m_pd3dcbPlayer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);
	// �Ҵ����ش�.
	m_pd3dcbPlayer->Map(0, NULL, (void **)&m_pcbMappedPlayer);
}

void CPlayer::ReleaseShaderVariables()
{
	if (m_pCamera) m_pCamera->ReleaseShaderVariables();

	if (m_pd3dcbPlayer)
	{
		m_pd3dcbPlayer->Unmap(0, NULL);
		m_pd3dcbPlayer->Release();
	}

	CGameObject::ReleaseShaderVariables();
}

void CPlayer::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
	XMStoreFloat4x4(&m_pcbMappedPlayer->m_xmf4x4World, XMMatrixTranspose(XMLoadFloat4x4(&m_xmf4x4World)));

	D3D12_GPU_VIRTUAL_ADDRESS d3dGpuVirtualAddress = m_pd3dcbPlayer->GetGPUVirtualAddress();
	pd3dCommandList->SetGraphicsRootConstantBufferView(0, d3dGpuVirtualAddress);
}

void CPlayer::Move(DWORD dwDirection, float fDistance, bool bUpdateVelocity)
{

	//
	if (dwDirection)
	{
		//xmf3Shift�� �׾߸��� �ش� ���������� �̵������̴�. 

		XMFLOAT3 xmf3Shift = XMFLOAT3(0, 0, 0);
		if (dwDirection & DIR_FORWARD) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, fDistance);	// �躤�� �������� fDistance��ŭ�� ���� xmfShift�� ���϶�. 
		if (dwDirection & DIR_BACKWARD) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, -fDistance);
		if (dwDirection & DIR_RIGHT) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, fDistance);
		if (dwDirection & DIR_LEFT) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, -fDistance);
		if (dwDirection & DIR_UP) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Up, fDistance);
		if (dwDirection & DIR_DOWN) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Up, -fDistance);

		//bUpdateVelocity �� true�ΰ��� �ڵ���ó�� �ӵ� ��ȭ�� �ִ°�쿡 ���������
		// true : �ӵ��� ���ӵ��� ���� ��, Shift�� ���� velocity�� ������ �ӵ� ��꿡 Ȱ���
		// false : �ӵ��� ���� ��ġ ��꿡 Ȱ���.(�ӵ� ����)
		Move(xmf3Shift,bUpdateVelocity);
	}
}

void CPlayer::Move(const XMFLOAT3& xmf3Shift, bool bUpdateVelocity)
{
	if (bUpdateVelocity)
	{
		m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, xmf3Shift);
	}
	else
	{
		m_xmf3Position = Vector3::Add(m_xmf3Position, xmf3Shift);
		this->m_sphereBox->Center.x = m_xmf3Position.x;
		this->m_sphereBox->Center.y = m_xmf3Position.y;
		this->m_sphereBox->Center.z = m_xmf3Position.z;

		// �÷��̾��� ��ġ ���ŵǸ鼭 
		//�÷��̾ ����ٴϴ� ī�޶��� ��ġ�� ���ŵ�.
		m_pCamera->Move(xmf3Shift);
	}
}

void CPlayer::Rotate(float x, float y, float z)
{

	//�÷��̾��� ȸ���� ����ϴ� �Լ�.
	//����Ī, ����Ī, spaceshipCamera���� ȸ���� �ٸ�
	DWORD nCurrentCameraMode = m_pCamera->GetMode();
	if ((nCurrentCameraMode == FIRST_PERSON_CAMERA) || (nCurrentCameraMode == THIRD_PERSON_CAMERA))
	{
		//1��Ī �̰ų� 3��Ī �϶��� �÷��̾� ȸ��
		if (x != 0.0f)
		{
			m_fPitch += x;
			if (m_fPitch > +89.0f) { x -= (m_fPitch - 89.0f); m_fPitch = +89.0f; }
			if (m_fPitch < -89.0f) { x -= (m_fPitch + 89.0f); m_fPitch = -89.0f; }
		}
		if (y != 0.0f)
		{
			m_fYaw += y;
			if (m_fYaw > 360.0f) m_fYaw -= 360.0f;
			if (m_fYaw < 0.0f) m_fYaw += 360.0f;
		}
		if (z != 0.0f)
		{
			m_fRoll += z;
			if (m_fRoll > +20.0f) { z -= (m_fRoll - 20.0f); m_fRoll = +20.0f; }
			if (m_fRoll < -20.0f) { z -= (m_fRoll + 20.0f); m_fRoll = -20.0f; }
		}
		m_pCamera->Rotate(x, y, z);
		if (y != 0.0f)
		{
			XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Up), XMConvertToRadians(y));
			m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
			m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
		}
	}
	else if (nCurrentCameraMode == SPACESHIP_CAMERA)
	{
		// ������ ��� == spaceShipCamera ����϶��� �÷��̾� ȸ��
		m_pCamera->Rotate(x, y, z);
		if (x != 0.0f)
		{
			XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Right), XMConvertToRadians(x));
			m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
			m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, xmmtxRotate);
		}
		if (y != 0.0f)
		{
			XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Up), XMConvertToRadians(y));
			m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
			m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
		}
		if (z != 0.0f)
		{
			XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Look), XMConvertToRadians(z));
			m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, xmmtxRotate);
			m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
		}
	}

	m_xmf3Look = Vector3::Normalize(m_xmf3Look);
	m_xmf3Right = Vector3::CrossProduct(m_xmf3Up, m_xmf3Look, true);
	m_xmf3Up = Vector3::CrossProduct(m_xmf3Look, m_xmf3Right, true);
}

void CPlayer::Update(float fTimeElapsed)
{

	//�÷��̾� ��ġ�� ���ŵǴ� �κ��� �̺κ��̴� . 

	// �ӵ��� �߷��� ���� ���ؼ� �ӵ��� ���� 
	m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, Vector3::ScalarProduct(m_xmf3Gravity, fTimeElapsed, false));


	float fLength = sqrtf(m_xmf3Velocity.x * m_xmf3Velocity.x + m_xmf3Velocity.z * m_xmf3Velocity.z);
	//�÷��̾� �ִ�ӵ� ����
	float fMaxVelocityXZ = m_fMaxVelocityXZ * fTimeElapsed;
	if (fLength > m_fMaxVelocityXZ)
	{
		m_xmf3Velocity.x *= (fMaxVelocityXZ / fLength);
		m_xmf3Velocity.z *= (fMaxVelocityXZ / fLength);
	}
	float fMaxVelocityY = m_fMaxVelocityY * fTimeElapsed;
	fLength = sqrtf(m_xmf3Velocity.y * m_xmf3Velocity.y);
	if (fLength > m_fMaxVelocityY)
	{
		m_xmf3Velocity.y *= (fMaxVelocityY / fLength);
	}
	Move(m_xmf3Velocity, false);


	//OnPlayerUpdateCallback�Լ��� virtual�� �Ǿ��ְ� 
	// ���������� CTerrainPlayer�� �츮�� ����ϱ� ������ 
	// CTerrainPlayer::OnPlayerUpdateCallback�Լ��� ��������Ѵ�.
	//�� �Լ��� ���� �÷��̾ �������� �ٴҼ� �ִ°�.
	if (m_pPlayerUpdatedContext) OnPlayerUpdateCallback(fTimeElapsed);

	DWORD nCurrentCameraMode = m_pCamera->GetMode();
	if (nCurrentCameraMode == THIRD_PERSON_CAMERA) m_pCamera->Update(m_xmf3Position, fTimeElapsed);
	if (m_pCameraUpdatedContext) OnCameraUpdateCallback(fTimeElapsed);
	if (nCurrentCameraMode == THIRD_PERSON_CAMERA) m_pCamera->SetLookAt(m_xmf3Position);
	m_pCamera->RegenerateViewMatrix();

	fLength = Vector3::Length(m_xmf3Velocity);
	float fDeceleration = (m_fFriction * fTimeElapsed);
	if (fDeceleration > fLength) fDeceleration = fLength;
	m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, Vector3::ScalarProduct(m_xmf3Velocity, -fDeceleration, true));
}

CCamera *CPlayer::OnChangeCamera(DWORD nNewCameraMode, DWORD nCurrentCameraMode)
{
	CCamera *pNewCamera = NULL;
	switch (nNewCameraMode)
	{
	case FIRST_PERSON_CAMERA:
		pNewCamera = new CFirstPersonCamera(m_pCamera);
		break;
	case THIRD_PERSON_CAMERA:
		pNewCamera = new CThirdPersonCamera(m_pCamera);
		break;
	case SPACESHIP_CAMERA:
		pNewCamera = new CSpaceShipCamera(m_pCamera);
		break;
	}
	if (nCurrentCameraMode == SPACESHIP_CAMERA)
	{
		m_xmf3Right = Vector3::Normalize(XMFLOAT3(m_xmf3Right.x, 0.0f, m_xmf3Right.z));
		m_xmf3Up = Vector3::Normalize(XMFLOAT3(0.0f, 1.0f, 0.0f));
		m_xmf3Look = Vector3::Normalize(XMFLOAT3(m_xmf3Look.x, 0.0f, m_xmf3Look.z));

		m_fPitch = 0.0f;
		m_fRoll = 0.0f;
		m_fYaw = Vector3::Angle(XMFLOAT3(0.0f, 0.0f, 1.0f), m_xmf3Look);
		if (m_xmf3Look.x < 0.0f) m_fYaw = -m_fYaw;
	}
	else if ((nNewCameraMode == SPACESHIP_CAMERA) && m_pCamera)
	{
		m_xmf3Right = m_pCamera->GetRightVector();
		m_xmf3Up = m_pCamera->GetUpVector();
		m_xmf3Look = m_pCamera->GetLookVector();
	}

	if (pNewCamera)
	{
		pNewCamera->SetMode(nNewCameraMode);
		pNewCamera->SetPlayer(this);
	}

	if (m_pCamera) delete m_pCamera;

	return(pNewCamera);
}

void CPlayer::OnPrepareRender()
{
	m_xmf4x4World._11 = m_xmf3Right.x; m_xmf4x4World._12 = m_xmf3Right.y; m_xmf4x4World._13 = m_xmf3Right.z;
	m_xmf4x4World._21 = m_xmf3Up.x; m_xmf4x4World._22 = m_xmf3Up.y; m_xmf4x4World._23 = m_xmf3Up.z;
	m_xmf4x4World._31 = m_xmf3Look.x; m_xmf4x4World._32 = m_xmf3Look.y; m_xmf4x4World._33 = m_xmf3Look.z;
	m_xmf4x4World._41 = m_xmf3Position.x; m_xmf4x4World._42 = m_xmf3Position.y; m_xmf4x4World._43 = m_xmf3Position.z;
}

void CPlayer::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	DWORD nCameraMode = (pCamera) ? pCamera->GetMode() : 0x00;
	if (nCameraMode == THIRD_PERSON_CAMERA) CGameObject::Render(pd3dCommandList, pCamera);
}




///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CTerrainPlayer
//
CTerrainPlayer::CTerrainPlayer(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, void *pContext, int nMeshes) : CPlayer(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pContext, nMeshes)
{
	//�÷��̾ �ް�ٴϴ� ī�޶��� �����ϸ� �ǰ���
	// ī�޶���� 1��Ī,3��Ī, ��������(SPACE_SHIP_CAMERA)�� ����
	m_pCamera = ChangeCamera(THIRD_PERSON_CAMERA, 0.0f);

	//�� �Լ��� ���̴����� ��� ����(���̴� ���� ����������� �����ϸ� ���ҵ�)�� �����ϰ� ���� �޸��� �޸� �Ҵ��� �ϴ� �Լ�
	//
	CreateShaderVariables(pd3dDevice, pd3dCommandList);	

	//�� ���������� �÷��̾�� �׸� ť�� �޽��̴�. 
	// �� �̺κп��� ĳ������ �޽��� �����ϰ� set���ش�. Diffused�� �ǹ̴� �ؽ��ĸ� ������ �ƴ� ���� �������� �̷�����ִٴ� �ǹ�
	CCubeMeshDiffused *pCubeMesh = new CCubeMeshDiffused(pd3dDevice, pd3dCommandList, 4.0f, 12.0f, 4.0f);	//�� �Լ����� ť���� ũ��� ������ ������ �����ش�.
	SetMesh(0, pCubeMesh);

	UINT ncbElementBytes = ((sizeof(CB_PLAYER_INFO) + 255) & ~255); //256�� ���


	//�÷��̾� �����ڰ� ���̴��� ���� �����͸� �����ִ� . 
	//�̷��� ������ �Ϲ��������� �ʴ� . 
	//���� ���� CShader�� ��ӹ޴� ��ü�� �ڽŵ��� ����Լ� BuildObjects ������ 
	//��ü�� �����ϴ� ��찡 �Ϲ����δ�. �����尰�� ��찡 �׷��� ������ �Ǿ��ִ�.
	//Terrain�̳� SkyBox�� ���� �� ���� ó�� Player�� ������ ������ �̷����ִ�.

	//�� Playerĳ������ ���� Shader�� �����ϰ� ������� ���� �����ϰ� ����
	CPlayerShader *pShader = new CPlayerShader();
	pShader->CreateShader(pd3dDevice, pd3dGraphicsRootSignature);		//�� �Լ��� Ÿ�� ���� CShader::CreateShader�Լ��� ȣ�� �ϰ� �ִ�. 
	pShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);
	pShader->CreateCbvAndSrvDescriptorHeaps(pd3dDevice, pd3dCommandList, 1, 0);
	pShader->CreateConstantBufferViews(pd3dDevice, pd3dCommandList, 1, m_pd3dcbPlayer, ncbElementBytes);




	SetCbvGPUDescriptorHandle(pShader->GetGPUCbvDescriptorStartHandle());

	//�÷��̾�� �����ڿ��� ���̴��� �����ϱ⶧���� SetShader�� ȣ���������
	SetShader(pShader);



	CHeightMapTerrain *pTerrain = (CHeightMapTerrain *)pContext;
	SetPosition(XMFLOAT3(pTerrain->GetWidth()*0.5f, 2000.0f, pTerrain->GetLength()*0.5f));

	//�÷��̾�� ���� ������ �˷������
	SetPlayerUpdatedContext(pTerrain);
	//ī�޶󿡰Ե� ���� ������ �˷������
	SetCameraUpdatedContext(pTerrain);




}

CTerrainPlayer::~CTerrainPlayer()
{
}

CCamera *CTerrainPlayer::ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed)
{
	// ���� ���Լ����� ������ �ִ� ī�޶� �����ϰ� ���ο� ī�޶� �Ҵ���

	DWORD nCurrentCameraMode = (m_pCamera) ? m_pCamera->GetMode() : 0x00;
	if (nCurrentCameraMode == nNewCameraMode) return(m_pCamera);
	switch (nNewCameraMode)
	{
		case FIRST_PERSON_CAMERA:
			SetFriction(250.0f);
			SetGravity(XMFLOAT3(0.0f, -400.0f, 0.0f));
			SetMaxVelocityXZ(300.0f);
			SetMaxVelocityY(400.0f);
			m_pCamera = OnChangeCamera(FIRST_PERSON_CAMERA, nCurrentCameraMode);	//�� �κп��� �ش� ī�޶� �Ҵ���
			m_pCamera->SetTimeLag(0.0f);
			m_pCamera->SetOffset(XMFLOAT3(0.0f, 20.0f, 0.0f));
			m_pCamera->GenerateProjectionMatrix(1.01f, 50000.0f, ASPECT_RATIO, 60.0f);
			break;
		case SPACESHIP_CAMERA:
			SetFriction(125.0f);
			SetGravity(XMFLOAT3(0.0f, 0.0f, 0.0f));
			SetMaxVelocityXZ(300.0f);
			SetMaxVelocityY(400.0f);
			m_pCamera = OnChangeCamera(SPACESHIP_CAMERA, nCurrentCameraMode);
			m_pCamera->SetTimeLag(0.0f);
			m_pCamera->SetOffset(XMFLOAT3(0.0f, 0.0f, 0.0f));
			m_pCamera->GenerateProjectionMatrix(1.01f, 50000.0f, ASPECT_RATIO, 60.0f);
			break;
		case THIRD_PERSON_CAMERA:
			SetFriction(250.0f);
			SetGravity(XMFLOAT3(0.0f, -250.0f, 0.0f));
			SetMaxVelocityXZ(300.0f);
			SetMaxVelocityY(400.0f);
			m_pCamera = OnChangeCamera(THIRD_PERSON_CAMERA, nCurrentCameraMode);
			m_pCamera->SetTimeLag(0.25f);
			m_pCamera->SetOffset(XMFLOAT3(0.0f, 20.0f, -50.0f));
			m_pCamera->GenerateProjectionMatrix(1.01f, 50000.0f, ASPECT_RATIO, 60.0f);
			break;
		default:
			break;
	}
	Update(fTimeElapsed);

	return(m_pCamera);
}

void CTerrainPlayer::OnPlayerUpdateCallback(float fTimeElapsed)
{

	
	CHeightMapTerrain *pTerrain = (CHeightMapTerrain *)m_pPlayerUpdatedContext;// ������ �����͸� �޾ƿ´�. 
	XMFLOAT3 xmf3Scale = pTerrain->GetScale();									//������ ũ�⸦ �޾ƿ´�.
	XMFLOAT3 xmf3PlayerPosition = GetPosition();								//�÷��̾� �ڽ��� ��ġ�� �޾ƿ´�.
	int z = (int)(xmf3PlayerPosition.z / xmf3Scale.z);							//������ 
	bool bReverseQuad = ((z % 2) != 0);
	float fHeight = pTerrain->GetHeight(xmf3PlayerPosition.x, xmf3PlayerPosition.z, bReverseQuad) + 6.0f; //�ͷ����� ��ġ�� �޾ƿ� height�� �Ѱ��ش�. +6�� ���� ������ �÷��̾��� pivot�� �� ����̱� ������
																											// ���� ��ġ�ϰ� �Ϸ��� �׸�ŭ ���������.

	

	//���� �� ���������� ������ �ͷ��� 1�� �ְ� ���� �����ϰ� ���� ������ �ͷ���2�� �ִ�. 
	// fHeight�� ���������� �ͷ����� ���� �����̰� �Ϸ��� �Ѱ��̰�
	// 220 ���� �� ������ ������ �ͷ����� ���� ���̰� 220�̱� �����̴�. 
	// 
	if (xmf3PlayerPosition.y <= 220.0f)
	{
		XMFLOAT3 xmf3PlayerVelocity = GetVelocity();
		xmf3PlayerVelocity.y = 0.0f;
		SetVelocity(xmf3PlayerVelocity);
		xmf3PlayerPosition.y = 220.0f;
		SetPosition(xmf3PlayerPosition);
	}
	
	//�ͷ��� 1�� ������ �÷��̾��� y��ġ�� fHeight���� �Ʒ��� ��������� fHeight�� �������ش�. 
	if (xmf3PlayerPosition.y < fHeight)
	{
		XMFLOAT3 xmf3PlayerVelocity = GetVelocity();
		xmf3PlayerVelocity.y = 0.0f;
		SetVelocity(xmf3PlayerVelocity);
		xmf3PlayerPosition.y = fHeight;
		SetPosition(xmf3PlayerPosition);

	}


}

//�÷��̾� �ڸ� ����ٴϴ� ī�޶� ������ �Ʒ��� �İ� �� ��� ���� ���� �÷������.
void CTerrainPlayer::OnCameraUpdateCallback(float fTimeElapsed)
{

	//ī�޶� CPlayer::OnPlayerUpdateCallback �̶� ����
	CHeightMapTerrain *pTerrain = (CHeightMapTerrain *)m_pCameraUpdatedContext;
	XMFLOAT3 xmf3Scale = pTerrain->GetScale();
	XMFLOAT3 xmf3CameraPosition = m_pCamera->GetPosition();
	int z = (int)(xmf3CameraPosition.z / xmf3Scale.z);
	bool bReverseQuad = ((z % 2) != 0);
	float fHeight = pTerrain->GetHeight(xmf3CameraPosition.x, xmf3CameraPosition.z, bReverseQuad) + 5.0f;
	if (xmf3CameraPosition.y <= 220.0f)
	{
		xmf3CameraPosition.y = 220.0f;
		m_pCamera->SetPosition(xmf3CameraPosition);
		if (m_pCamera->GetMode() == THIRD_PERSON_CAMERA)
		{
			CThirdPersonCamera *p3rdPersonCamera = (CThirdPersonCamera *)m_pCamera;
			p3rdPersonCamera->SetLookAt(GetPosition());
		}
	}
	if (xmf3CameraPosition.y < fHeight)
	{
		xmf3CameraPosition.y = fHeight;
		m_pCamera->SetPosition(xmf3CameraPosition);
		if (m_pCamera->GetMode() == THIRD_PERSON_CAMERA)
		{
			CThirdPersonCamera *p3rdPersonCamera = (CThirdPersonCamera *)m_pCamera;
			p3rdPersonCamera->SetLookAt(GetPosition());
		}
	}
}
