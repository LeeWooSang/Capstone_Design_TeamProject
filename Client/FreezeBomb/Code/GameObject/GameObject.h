#pragma once

#include "../Mesh/Mesh.h"
#include "../Camera/Camera.h"

#define ANIMATION_TYPE_ONCE			0
#define ANIMATION_TYPE_LOOP			1
#define ANIMATION_TYPE_PINGPONG	2

#define ANIMATION_CALLBACK_EPSILON	0.015f

struct CALLBACKKEY
{
	// �ð�
   float  				m_fTime = 0.0f;
   // ȿ���� ���� �Ҹ� => ���ڱ��Ҹ��� ����� ������
   void*				m_pCallbackData = NULL;
};

struct KEYFRAME
{
   float  				m_fTime = 0.0f;
   float  				m_fValue = 0.0f;
};

class CAnimationCurve
{
public:
	CAnimationCurve() { }
	~CAnimationCurve() { }

public:
   int  				m_nKeyFrames = 0;
   KEYFRAME	*	m_pKeyFrames = NULL;
};

//#define _WITH_ANIMATION_SRT
#define _WITH_ANIMATION_INTERPOLATION

class CAnimationCallbackHandler
{
public:
	CAnimationCallbackHandler() { }
	~CAnimationCallbackHandler() { }

public:
   virtual void HandleCallback(void *pCallbackData) { }
};

class CAnimationSet
{
public:
	CAnimationSet();
	~CAnimationSet();

public:
	char											m_pstrName[64];

	// �ִϸ��̼� ����� ���� ������ �����Ѵ�.  => �ð�
	float											m_fLength = 0.0f;
	int												m_nFramesPerSecond = 0; //m_fTicksPerSecond

	// �ִϸ��̼��� ���� ������ ���� => �ִϸ��̼� ���
	int												m_nKeyFrameTransforms = 0;
	float*										m_pfKeyFrameTransformTimes = NULL;
	XMFLOAT4X4**						m_ppxmf4x4KeyFrameTransforms = NULL;

#ifdef _WITH_ANIMATION_SRT
	int												m_nKeyFrameScales = 0;
	float*										m_pfKeyFrameScaleTimes = NULL;
	XMFLOAT3**							m_ppxmf3KeyFrameScales = NULL;
	int												m_nKeyFrameRotations = 0;
	float*										m_pfKeyFrameRotationTimes = NULL;
	XMFLOAT4**							m_ppxmf4KeyFrameRotations = NULL;
	int												m_nKeyFrameTranslations = 0;
	float*										m_pfKeyFrameTranslationTimes = NULL;
	XMFLOAT3**							m_ppxmf3KeyFrameTranslations = NULL;
#endif

	float 											m_fPosition = 0.0f;
    int 											m_nType = ANIMATION_TYPE_LOOP; //Once, Loop, PingPong

	int												m_nCurrentKey = -1;

	int 											m_nCallbackKeys = 0;
	CALLBACKKEY*							m_pCallbackKeys = NULL;

	CAnimationCallbackHandler*	m_pAnimationCallbackHandler = NULL;

public:
	// AnimationTrack�� �����ǰ� => �ִϸ��̼ǿ��� �о���ϴ� ��ġ => ���� �ٸ������� �ϰ���
	void SetPosition(float fTrackPosition);

	XMFLOAT4X4 GetSRT(int nFrame);

	void SetCallbackKeys(int nCallbackKeys);
	void SetCallbackKey(int nKeyIndex, float fTime, void *pData);
	void SetAnimationCallbackHandler(CAnimationCallbackHandler *pCallbackHandler);

	void *GetCallbackData();
};

class CGameObject;

class CAnimationSets
{
private:
	int								m_nReferences = 0;

public:
	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }

public:
	CAnimationSets();
	~CAnimationSets();

public:
	int							m_nAnimationFrames = 0; 
	CGameObject**	m_ppAnimationFrameCaches = NULL;

	int							m_nAnimationSets = 0;
	CAnimationSet*	m_pAnimationSets = NULL;

public:
	void SetCallbackKeys(int nAnimationSet, int nCallbackKeys);
	void SetCallbackKey(int nAnimationSet, int nKeyIndex, float fTime, void *pData);
	void SetAnimationCallbackHandler(int nAnimationSet, CAnimationCallbackHandler *pCallbackHandler);
};

class CAnimationTrack
{
public:
	CAnimationTrack() { }
	~CAnimationTrack() { }

public:
	// �ִϸ��̼��� ���� ������ �����ϴ� �����ϴ� ����
    BOOL 						m_bEnable = true;
	// �ִϸ��̼� ��� �ӵ�
    float 							m_fSpeed = 1.0f;
	// �ִϸ��̼��� ��� ��ġ���� �о���ϴ��� ���� ����
    float 							m_fPosition = 0.0f;
	// ������ �ִϸ��̼��� �����Ͽ�, ���ο� �ִϸ��̼��� ����� ���� ������
    float 							m_fWeight = 1.0f;
	// � �ִϸ��̼��� �������� ������
    CAnimationSet*		m_pAnimationSet = NULL;

public:
	void SetAnimationSet(CAnimationSet *pAnimationSet) { m_pAnimationSet = pAnimationSet; }
	void SetEnable(bool bEnable) { m_bEnable = bEnable; }
	void SetSpeed(float fSpeed) { m_fSpeed = fSpeed; }
	void SetWeight(float fWeight) { m_fWeight = fWeight; }
	void SetPosition(float fPosition) { m_fPosition = fPosition; }
};

class CAnimationController 
{
public:
	CAnimationController(int nAnimationTracks, CAnimationSets *pAnimationSets);
	~CAnimationController();

public:
    float 							m_fTime = 0.0f;

	CAnimationSets*		m_pAnimationSets = NULL;

    int 							m_nAnimationTracks = 0;
    CAnimationTrack*	m_pAnimationTracks = NULL;

public:
	void SetAnimationSets(CAnimationSets *pAnimationSets);

	void SetTrackAnimationSet(int nAnimationTrack, int nAnimationSet);
	void SetTrackEnable(int nAnimationTrack, bool bEnable);
	void SetTrackPosition(int nAnimationTrack, float fPosition);
	void SetTrackSpeed(int nAnimationTrack, float fSpeed);
	void SetTrackWeight(int nAnimationTrack, float fWeight);

	void SetCallbackKeys(int nAnimationSet, int nCallbackKeys);
	void SetCallbackKey(int nAnimationSet, int nKeyIndex, float fTime, void *pData);
	void SetAnimationCallbackHandler(int nAnimationSet, CAnimationCallbackHandler *pCallbackHandler);

	void AdvanceTime(float fElapsedTime);
};

class CLoadedModelInfo
{
public:
	CLoadedModelInfo() { }
	~CLoadedModelInfo() { }

	int 						m_nSkinnedMeshes = 0;
	int							m_nFrameMeshes = 0;

    CGameObject*		m_pModelRootObject = NULL;
	CAnimationSets*	m_pAnimationSets = NULL;
};

class CSkinningBoneTransforms
{
public:
	CSkinningBoneTransforms(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CLoadedModelInfo *pModel);
	~CSkinningBoneTransforms();

public:
	int 							m_nSkinnedMeshes = 0;

	CSkinnedMesh**		m_ppSkinnedMeshes = NULL;

	ID3D12Resource**	m_ppd3dcbBoneTransforms = NULL;
	XMFLOAT4X4**		m_ppcbxmf4x4BoneTransforms = NULL;

public:
	void SetSkinnedMesh(int nIndex, CSkinnedMesh *pSkinnedMesh) { m_ppSkinnedMeshes[nIndex] = pSkinnedMesh; }

	void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	void ReleaseShaderVariables();

	void SetSkinnedMeshBoneTransformConstantBuffer();
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define DIR_FORWARD			0x01
#define DIR_BACKWARD			0x02
#define DIR_LEFT						0x04
#define DIR_RIGHT					0x08
#define DIR_UP						0x10
#define DIR_DOWN					0x20

class CTexture;
class CMaterial;
class CShader;
class CCarry;
class CLampParticle;
class CGameObject
{
private:
	int								m_nReferences = 0;

public:
	void AddRef();
	void Release();

public:
	CGameObject();
	CGameObject(int nMaterials);
    virtual ~CGameObject();

public:
	char					m_pstrFrameName[64];

	CMesh*			m_pMesh = NULL;

	int						m_nMaterials = 0;
	CMaterial**		m_ppMaterials = NULL;

	XMFLOAT4X4	m_xmf4x4ToParent;
	XMFLOAT4X4	m_xmf4x4World;

	CGameObject*	m_pParent = NULL;
	CGameObject*	m_pChild = NULL;
	CGameObject*	m_pSibling = NULL;

	void SetMesh(CMesh *pMesh);
	void SetShader(CShader *pShader);
	void SetShader(int nMaterial, CShader *pShader);
	void SetMaterial(int nMaterial, CMaterial *pMaterial);

	void SetChild(CGameObject *pChild, bool bReferenceUpdate=false);

	virtual void BuildMaterials(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList) { }

	virtual void OnPrepareAnimate() { }
	virtual void Animate(float fTimeElapsed);

	virtual void OnPrepareRender() { }
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera=NULL);

	virtual void ReleaseShaderVariables();

	virtual void UpdateShaderVariable(ID3D12GraphicsCommandList *pd3dCommandList, XMFLOAT4X4 *pxmf4x4World);
	virtual void UpdateShaderVariable(ID3D12GraphicsCommandList *pd3dCommandList, CMaterial *pMaterial);

	virtual void ReleaseUploadBuffers();

	XMFLOAT3 GetPosition();
	XMFLOAT3 GetLook();
	XMFLOAT3 GetUp();
	XMFLOAT3 GetRight();

	void SetPosition(float x, float y, float z);
	void SetPosition(XMFLOAT3 xmf3Position);
	void SetScale(float x, float y, float z);

	void MoveStrafe(float fDistance = 1.0f);
	void MoveUp(float fDistance = 1.0f);
	void MoveForward(float fDistance = 1.0f);

	void Rotate(float fPitch = 10.0f, float fYaw = 10.0f, float fRoll = 10.0f);
	void Rotate(XMFLOAT3 *pxmf3Axis, float fAngle);
	void Rotate(XMFLOAT4 *pxmf4Quaternion);

	CGameObject* GetParent() { return(m_pParent); }
	void UpdateTransform(XMFLOAT4X4 *pxmf4x4Parent=NULL);
	CGameObject* FindFrame(char *pstrFrameName);

	CTexture* FindReplicatedTexture(_TCHAR *pstrTextureName);

	UINT GetMeshType() { return((m_pMesh) ? m_pMesh->GetType() : 0x00); }

public:
	// ������ ��ü�� AnimationController�� ������ �־, ���δٸ� ������ �� �� �ֵ��� ����.
	CAnimationController*			m_pAnimationController = NULL;
	// �޽��� �ƴ� ������ ��ü�� transform ����� ���� �ֱ� ���� => �𵨰������� �ذ�
	CSkinningBoneTransforms*	m_pSkinningBoneTransforms = NULL;

	CGameObject* GetRootSkinnedGameObject();

	void SetTrackAnimationSet(int nAnimationTrack, int nAnimationSet);
	void SetTrackAnimationPosition(int nAnimationTrack, float fPosition);

	void CacheSkinningBoneFrames(CGameObject *pRootFrame);

	void FindAndSetSkinnedMesh(int *pnSkinMesh, CSkinningBoneTransforms *pSkinningBoneTransforms);

	void LoadMaterialsFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CGameObject *pParent, FILE *pInFile, CShader *pShader);

	static CAnimationSets *LoadAnimationFromFile(FILE *pInFile, CGameObject *pRootFrame);
	static CGameObject *LoadFrameHierarchyFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, 
		CGameObject *pParent, FILE *pInFile, CShader *pShader, int *pnSkinnedMeshes, int* pnFrameMeshes);

	static CLoadedModelInfo* LoadGeometryAndAnimationFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, char *pstrFileName, CShader *pShader, bool bHasAnimation);

	static void PrintFrameInfo(CGameObject *pGameObject, CGameObject *pParent);

protected:
	CCarry*					m_pCarry{ nullptr };
	CLampParticle*		m_pLampParticle{ nullptr };	
};