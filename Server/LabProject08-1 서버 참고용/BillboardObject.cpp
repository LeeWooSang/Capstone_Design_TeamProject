#include "stdafx.h"
#include "BillboardObject.h"


CBillboardObject::CBillboardObject()
{
}


CBillboardObject::~CBillboardObject()
{
}

void CBillboardObject::Animate(float fTimeElapsed, CCamera* pCamera,CPlayer *pPlayer)
{

	// ī�޶� ��ġ�� �޾ƿ´�.
	XMFLOAT3 xmf3CameraPosition = pCamera->GetPosition();
	//SetLookAt �Լ��� ����ϸ� ���ڷ� �Ѱ��� ��ġ�� �ٶ󺸰� �� �� �ִ�.
	SetLookAt(xmf3CameraPosition);
}

void CBillboardObject::SetLookAt(XMFLOAT3& xmfTarget)
{
	//�� ���� �������� ��ġ�� ��
	XMFLOAT3 xmf3Position(m_xmf4x4World._41, m_xmf4x4World._42, m_xmf4x4World._43);

	//�� ���� �������� UP���ʹ� �׻� ���� �ٶ󺸰� �־���Ѵ�. X,Y =1.0, Z������
	XMFLOAT3 xmf3Up(0.0f, 1.0f, 0.0f);

	//Look���͸� ���ϴ� �κ��̴�. 
// Vector3::SubtractNormalize (A=Ÿ��,B=������ ��ġ)	//Look����=A-B
// A-B => B�� A�� �ٶ󺸴� ���͸� ������. 
	XMFLOAT3 xmf3Look = Vector3::SubtractNormalize(xmfTarget, xmf3Position, true);
	// �躤�Ϳ� Up���͸� �����ϸ� Right���͸� ������ �ִ�.
	// �޼� ��ǥ������ ���� �ٶ�
	XMFLOAT3 xmf3Right = Vector3::CrossProduct(xmf3Up, xmf3Look, true);



	// �� ���� ���͸� ����� ������ �ִ´�.
// 1�� : Right����
// 2�� : up ����
// 3�� : Look����
// 4�� : ��ġ ���� (���⼭�� ��ġ���ʹ� �������� ����)
	m_xmf4x4World._11 = xmf3Right.x;
	m_xmf4x4World._12 = xmf3Right.y;
	m_xmf4x4World._13 = xmf3Right.z;
	m_xmf4x4World._21 = xmf3Up.x;
	m_xmf4x4World._22 = xmf3Up.y;
	m_xmf4x4World._23 = xmf3Up.z;
	m_xmf4x4World._31 = xmf3Look.x;
	m_xmf4x4World._32 = xmf3Look.y;
	m_xmf4x4World._33 = xmf3Look.z;

}

void CBillboardObject::Render(ID3D12GraphicsCommandList *pd3dCommandList,int objNum ,CCamera *pCamera)
{
	CGameObject::Render(pd3dCommandList, pCamera,objNum);
}


//////////////////////////////////////////////////////
//CGrassBillboardObject

CGrassBillboardObject::CGrassBillboardObject()
{
	totalTime = 0.0f;
}

CGrassBillboardObject::~CGrassBillboardObject()
{

}

//Ǯ ������ Animate�κ�
void CGrassBillboardObject::Animate(float fTimeElapsed, CCamera* pCamera,bool isIntersect)
{

	// ī�޶� ��ġ�� �޾ƿ´�.
	XMFLOAT3 xmf3CameraPosition = pCamera->GetPosition();
	
	//SetLookAt �Լ��� ����ϸ� ���ڷ� �Ѱ��� ��ġ�� �ٶ󺸰� �� �� �ִ�.
	SetLookAt(xmf3CameraPosition);
	
	// �� �κ��� Ǯ�� �¿�� ��鸮�Բ� �س��� �κ��̴�. 
	// �÷��̾� ������ 100m �̳��� ������ Ǯ�� ���� ���ݸ� ��鸰��.
	// �� �ۿ� ��ġ�� Ǯ ������� ���� ��鸰��. 


	totalTime += sinf(fTimeElapsed);
	float fAngle = sinf(0.5f*totalTime);

	XMFLOAT4X4 mtxRotate = Matrix4x4::Identity();


	// 
	if (isIntersect) {
		fAngle = 1.5f*cosf(3.0f*totalTime);
		//Ǯ ������Ʈ��  �÷��̾ �ٶ󺸰� �ִ� �����̹Ƿ� Rollȸ���� �ؾ���
		mtxRotate = Matrix4x4::RotationYawPitchRoll(0.0f, 0.0f, fAngle);
	}
	else {
		mtxRotate = Matrix4x4::RotationYawPitchRoll(0.0f, 0.0f, fAngle*60.0f);
	}

	//������ ���� ȸ����� mtxRotate�� ���� �ڽ��� ���� ��ȯ ��ɿ� ���Ѵ�.
	// ���⼭ ���������� �ִ�.
	// T: �̵� ���
	// R : ȸ�� ���
	// �׻� �̵�����̶� ȸ������� ���������� ������
	// R(3) *R(2) *R(1) * T  �̷������� 
	// �̵������ �׻� ���������� ���������� .
	// �׷��� �ʰ� �߰��� T����� ���� ��� ������ �ƴϰ� ������ �ǹ���.
	m_xmf4x4World = Matrix4x4::Multiply(mtxRotate, m_xmf4x4World);


}

void CGrassBillboardObject::SetLookAt(XMFLOAT3& xmfTarget)
{

	//�� Ǯ �������� ��ġ�� ��
	XMFLOAT3 xmf3Position(m_xmf4x4World._41, m_xmf4x4World._42, m_xmf4x4World._43);

	//�� Ǯ �������� UP���ʹ� �׻� ���� �ٶ󺸰� �־���Ѵ�. X,Y =1.0, Z������
	XMFLOAT3 xmf3Up(0.0f, 1.0f, 0.0f);
	//Look���͸� ���ϴ� �κ��̴�. 
	// Vector3::SubtractNormalize (A=Ÿ��,B=������ ��ġ)	//Look����=A-B
	// A-B => B�� A�� �ٶ󺸴� ���͸� ������. 
	XMFLOAT3 xmf3Look = Vector3::SubtractNormalize(xmfTarget, xmf3Position, true);
	
	// �躤�Ϳ� Up���͸� �����ϸ� Right���͸� ������ �ִ�.
	// �޼� ��ǥ������ ���� �ٶ�
	XMFLOAT3 xmf3Right = Vector3::CrossProduct(xmf3Up, xmf3Look, true);



	// �� ���� ���͸� ����� ������ �ִ´�.
	// 1�� : Right����
	// 2�� : up ����
	// 3�� : Look����
	// 4�� : ��ġ ���� (���⼭�� ��ġ���ʹ� �������� ����)

	m_xmf4x4World._11 = xmf3Right.x;
	m_xmf4x4World._12 = xmf3Right.y;
	m_xmf4x4World._13 = xmf3Right.z;
	m_xmf4x4World._21 = xmf3Up.x;
	m_xmf4x4World._22 = xmf3Up.y;
	m_xmf4x4World._23 = xmf3Up.z;
	m_xmf4x4World._31 = xmf3Look.x;
	m_xmf4x4World._32 = xmf3Look.y;
	m_xmf4x4World._33 = xmf3Look.z;


}

void CGrassBillboardObject::Render(ID3D12GraphicsCommandList *pd3dCommandList,int objNum, CCamera *pCamera)
{
	CGameObject::Render(pd3dCommandList, pCamera,objNum);
}


