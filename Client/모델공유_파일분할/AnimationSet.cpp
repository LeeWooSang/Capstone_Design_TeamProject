#include "stdafx.h"
#include "AnimationSet.h"

CAnimationSet::CAnimationSet()
{
}

CAnimationSet::~CAnimationSet()
{
	if (m_pfKeyFrameTransformTimes) delete[] m_pfKeyFrameTransformTimes;
	for (int j = 0; j < m_nKeyFrameTransforms; j++) if (m_ppxmf4x4KeyFrameTransforms[j]) delete[] m_ppxmf4x4KeyFrameTransforms[j];
	if (m_ppxmf4x4KeyFrameTransforms) delete[] m_ppxmf4x4KeyFrameTransforms;

	if (m_pCallbackKeys) delete[] m_pCallbackKeys;
}

// ��Ȯ�� �ð��� ���� �ִϸ��̼��� �ϰ� �ؾߵȴ�.
// �ֳ��ϸ�, 1.5���� �ִϸ��̼ǵ� 1.6�ʿ� �ִϸ��̼��� �� �� �� �־ �ð��� �ȸ��� �� �ֱ� ������
float CAnimationSet::GetPosition(float fPosition)
{
	float fGetPosition = fPosition;
	// �ִϸ��̼� Ÿ�Կ� ���� ������
	switch (m_nType)
	{
	case ANIMATION_TYPE_LOOP:
	{
		fGetPosition = fPosition - int(fPosition / m_pfKeyFrameTransformTimes[m_nKeyFrameTransforms - 1]) * m_pfKeyFrameTransformTimes[m_nKeyFrameTransforms - 1];
		//			fGetPosition = fPosition - int(fPosition / m_fLength) * m_fLength;
#ifdef _WITH_ANIMATION_INTERPOLATION			
#else
		m_nCurrentKey++;
		// ������ ���� �ʰ�, �ε����� �о �ִϸ��̼� �ϴ� ���
		// ��ſ� �����ӷ���Ʈ�� ���� �����ӷ���Ʈ�� ������ ���� �ִϸ��̼ǵǰ�, �����ӷ���Ʈ�� ������ �ִϸ��̼��� ������ �Ǵ� ������ ����
		// ���� �����ӷ���Ʈ�� �������Ѽ� ����ؾ��Ѵ�.
		if (m_nCurrentKey >= m_nKeyFrameTransforms) m_nCurrentKey = 0;
#endif
		break;
	}
	case ANIMATION_TYPE_ONCE:
		break;
	case ANIMATION_TYPE_PINGPONG:
		break;
	}
	return(fGetPosition);
}

#define _WITH_ITERATION

XMFLOAT4X4 CAnimationSet::GetSRT(int nFrame, float fPosition)
{
	XMFLOAT4X4 xmf4x4Transform = Matrix4x4::Identity();
#ifdef _WITH_ANIMATION_INTERPOLATION
#ifdef _WITH_ANIMATION_SRT
	XMVECTOR S, R, T;
	for (int i = 0; i < (m_nKeyFrameTranslations - 1); i++)
	{
		if ((m_pfKeyFrameTranslationTimes[i] <= fPosition) && (fPosition <= m_pfKeyFrameTranslationTimes[i + 1]))
		{
			float t = (fPosition - m_pfKeyFrameTranslationTimes[i]) / (m_pfKeyFrameTranslationTimes[i + 1] - m_pfKeyFrameTranslationTimes[i]);
			T = XMVectorLerp(XMLoadFloat3(&m_ppxmf3KeyFrameTranslations[i][nFrame]), XMLoadFloat3(&m_ppxmf3KeyFrameTranslations[i + 1][nFrame]), t);
			break;
		}
	}
	for (UINT i = 0; i < (m_nKeyFrameScales - 1); i++)
	{
		if ((m_pfKeyFrameScaleTimes[i] <= fPosition) && (fPosition <= m_pfKeyFrameScaleTimes[i + 1]))
		{
			float t = (fPosition - m_pfKeyFrameScaleTimes[i]) / (m_pfKeyFrameScaleTimes[i + 1] - m_pfKeyFrameScaleTimes[i]);
			S = XMVectorLerp(XMLoadFloat3(&m_ppxmf3KeyFrameScales[i][nFrame]), XMLoadFloat3(&m_ppxmf3KeyFrameScales[i + 1][nFrame]), t);
			break;
		}
	}
	for (UINT i = 0; i < (m_nKeyFrameRotations - 1); i++)
	{
		if ((m_pfKeyFrameRotationTimes[i] <= fPosition) && (fPosition <= m_pfKeyFrameRotationTimes[i + 1]))
		{
			float t = (fPosition - m_pfKeyFrameRotationTimes[i]) / (m_pfKeyFrameRotationTimes[i + 1] - m_pfKeyFrameRotationTimes[i]);
			R = XMQuaternionSlerp(XMLoadFloat4(&m_ppxmf4KeyFrameRotations[i][nFrame]), XMLoadFloat4(&m_ppxmf4KeyFrameRotations[i + 1][nFrame]), t);
			break;
		}
	}

	XMStoreFloat4x4(&xmf4x4Transform, XMMatrixAffineTransformation(S, NULL, R, T));
#else   
	for (int i = 0; i < (m_nKeyFrameTransforms - 1); i++)
	{
		if ((m_pfKeyFrameTransformTimes[i] <= fPosition) && (fPosition <= m_pfKeyFrameTransformTimes[i + 1]))
		{
			// i + 1���� i�� �� �� ���̸�ŭ �������� ������.
			float t = (fPosition - m_pfKeyFrameTransformTimes[i]) / (m_pfKeyFrameTransformTimes[i + 1] - m_pfKeyFrameTransformTimes[i]);
			XMVECTOR S0, R0, T0, S1, R1, T1;
			XMMatrixDecompose(&S0, &R0, &T0, XMLoadFloat4x4(&m_ppxmf4x4KeyFrameTransforms[i][nFrame]));
			XMMatrixDecompose(&S1, &R1, &T1, XMLoadFloat4x4(&m_ppxmf4x4KeyFrameTransforms[i + 1][nFrame]));
			// XMVectorLerp : ���� ���� ���� �Լ�
			XMVECTOR S = XMVectorLerp(S0, S1, t);
			XMVECTOR T = XMVectorLerp(T0, T1, t);
			// XMQuaternionSlerp : ���ʹϾ� ����
			XMVECTOR R = XMQuaternionSlerp(R0, R1, t);
			XMStoreFloat4x4(&xmf4x4Transform, XMMatrixAffineTransformation(S, XMVectorZero(), R, T));
			break;
		}
	}
#endif
#else
	xmf4x4Transform = m_ppxmf4x4KeyFrameTransforms[m_nCurrentKey][nFrame];
#endif
	return(xmf4x4Transform);
}

void CAnimationSet::SetCallbackKeys(int nCallbackKeys)
{
	m_nCallbackKeys = nCallbackKeys;
	m_pCallbackKeys = new CALLBACKKEY[nCallbackKeys];
}

void CAnimationSet::SetCallbackKey(int nKeyIndex, float fKeyTime, void *pData)
{
	m_pCallbackKeys[nKeyIndex].m_fTime = fKeyTime;
	m_pCallbackKeys[nKeyIndex].m_pCallbackData = pData;
}