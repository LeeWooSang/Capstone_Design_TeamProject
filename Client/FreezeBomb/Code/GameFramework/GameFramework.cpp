#include "../Stdafx/Stdafx.h"
#include "GameFramework.h"

// 메모리 릭이 있는지 알려준다.
#include <crtdbg.h>


#include "../Direct2D/Direct2D.h"

#include "../Network/Network.h"

#include "../Scene/Scene.h"
#include "../GameObject/Player/Player.h"
#include "../Scene/LobbyScene/LobbyScene.h"
#include "../Scene/LoadingScene/LoadingScene.h"
#include "../Scene/LoginScene/IPScene.h"
#include "../ShaderManager/ShaderManager.h"
#include "../Shader/TerrainShader/TerrainShader.h"
#include "../GameObject/Terrain/Terrain.h"
#include "../ResourceManager/ResourceManager.h"
#include "../Shader/StandardShader/MapToolShader/MapToolShader.h"
#include "../Shader/StandardShader/MapObjectShader/MapObjectShader.h"
#include "../Texture/Texture.h"
#include "../Shader/PostProcessShader/CartoonShader/SobelCartoonShader.h"
#include "../Chatting/Chatting.h"
#include "../Shader/StandardShader/SkinnedAnimationShader/SkinnedAnimationObjectShader/SkinnedAnimationObjectShader.h"
#include "../Shader/BillboardShader/UIShader/TimerUIShader/TimerUIShader.h"
#include "../Shader/BillboardShader/UIShader/CharacterSelectUIShader/CharacterSelectUIShader.h"
#include "../Shader/BillboardShader/BombParticleShader/BombParticleShader.h"
#include "../Shader/CubeParticleShader/ExplosionParticleShader/ExplosionParticleShader.h"
#include "../Scene/LoginScene/IDScene/LoginScene.h"
#include "../InputSystem/IDInputSystem/IDInputSystem.h"
#include "../Shader/BillboardShader/UIShader/LoginShader/IDShader.h"
#include "../GameObject/Item/Item.h"
#include "../GameObject/Billboard/Bomb/Bomb.h"
#include "../GameObject/EvilBear/EvilBear.h"
#include "../Shader/StandardShader/ItemShader/ItemShader.h"

#include "../ResourceManager/ResourceManager.h"
#include "../SoundSystem/SoundSystem.h"



ID2D1DeviceContext2*	CGameFramework::m_pd2dDeviceContext = nullptr;
IWICImagingFactory* CGameFramework::m_pwicImagingFactory = nullptr;
IDWriteFactory5* CGameFramework::m_pdWriteFactory = nullptr;
bool	CGameFramework::m_IceChangeOk = false;
map<int, clientsInfo> CGameFramework::m_mapClients;

#ifdef _WITH_SERVER_
char CGameFramework::m_HostID = -1;
#endif
// 전체모드할경우 주석풀으셈
#define FullScreenMode

//게임 상태 
int g_State = GAMESTATE::ID_INPUT;

bool g_OnCartoonShading = false;
bool g_IsSoundOn = true;

byte g_PlayerCharacter = CGameObject::PINK;

extern volatile size_t g_TotalSize;
extern volatile size_t g_FileSize;
extern volatile bool g_IsloadingStart;


unsigned char g_Round = 0;

#ifdef _WITH_SERVER_
//extern volatile bool g_LoginFinished;
volatile HWND g_hWnd;
#endif

CGameFramework::CGameFramework()
{
	m_pdxgiFactory6 = NULL;
	//m_pdxgiFactory = NULL;
	m_pdxgiSwapChain = NULL;
	m_pd3dDevice = NULL;

	for (int i = 0; i < m_nSwapChainBuffers; i++) m_ppd3dSwapChainBackBuffers[i] = NULL;
	m_nSwapChainBufferIndex = 0;

	m_pd3dCommandAllocator = NULL;
	m_pd3dCommandQueue = NULL;
	m_pd3dCommandList = NULL;

	m_pLoadingCommandAllocator = nullptr;
	m_pLoadingCommandList = nullptr;

	m_pd3dRtvDescriptorHeap = NULL;
	m_pd3dDsvDescriptorHeap = NULL;

	m_nRtvDescriptorIncrementSize = 0;
	m_nDsvDescriptorIncrementSize = 0;

	m_hFenceEvent = NULL;
	m_pd3dFence = NULL;
	for (int i = 0; i < m_nSwapChainBuffers; i++) m_nFenceValues[i] = 0;

	m_nWndClientWidth = FRAME_BUFFER_WIDTH;
	m_nWndClientHeight = FRAME_BUFFER_HEIGHT;

	m_pScene = NULL;
	m_pPlayer = NULL;

	_tcscpy_s(m_pszFrameRate, _T("FreezeBomb ("));

	// 메모리 릭이 있는지 체크를 해준다.
	// 릭이 있으면, 번호를 출력해준다.
	//_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	// 출력된 번호를 넣어주면 그 지점으로 바로 이동시켜준다.
	// [ 예시 ]
	// Detected memory leaks!
	//	Dumping objects ->
	// {233} normal block at 0x000001469D91A680, 24 bytes long.
	// 233 이라는 지점에서 릭이 생김	

	//_CrtSetBreakAlloc(1727);

}

CGameFramework::~CGameFramework()
{
}

bool CGameFramework::OnCreate(HINSTANCE hInstance, HWND hMainWnd)
{
	m_hInstance = hInstance;
	m_hWnd = hMainWnd;

	CreateDirect3DDevice();
	CreateCommandQueueAndList();
	CreateLoadingCommandList();

#ifdef _WITH_DIRECT2D_
	CreateDirect2DDevice();
#endif
	CreateRtvAndDsvDescriptorHeaps();
	CreateSwapChain();
	CreateDepthStencilView();

	if (BuildObjects() == false)
		return false;

	CreateOffScreenRenderTargetViews();

	return true;
}

#define _WITH_CREATE_SWAPCHAIN_FOR_HWND
void CGameFramework::CreateSwapChain()
{
	RECT rcClient;
	::GetClientRect(m_hWnd, &rcClient);
	m_nWndClientWidth = rcClient.right - rcClient.left;
	m_nWndClientHeight = rcClient.bottom - rcClient.top;


#ifdef _WITH_CREATE_SWAPCHAIN_FOR_HWND
	DXGI_SWAP_CHAIN_DESC1 dxgiSwapChainDesc;
	::ZeroMemory(&dxgiSwapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC1));
	dxgiSwapChainDesc.Width = m_nWndClientWidth;
	dxgiSwapChainDesc.Height = m_nWndClientHeight;
	dxgiSwapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiSwapChainDesc.SampleDesc.Count = (m_bMsaa4xEnable) ? 4 : 1;
	dxgiSwapChainDesc.SampleDesc.Quality = (m_bMsaa4xEnable) ? (m_nMsaa4xQualityLevels - 1) : 0;
	dxgiSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	dxgiSwapChainDesc.BufferCount = m_nSwapChainBuffers;
	dxgiSwapChainDesc.Scaling = DXGI_SCALING_NONE;
	dxgiSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	dxgiSwapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	dxgiSwapChainDesc.Flags =  DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	//dxgiSwapChainDesc.Flags = 

	DXGI_SWAP_CHAIN_FULLSCREEN_DESC dxgiSwapChainFullScreenDesc;
	::ZeroMemory(&dxgiSwapChainFullScreenDesc, sizeof(DXGI_SWAP_CHAIN_FULLSCREEN_DESC));
	dxgiSwapChainFullScreenDesc.RefreshRate.Numerator = 60;
	dxgiSwapChainFullScreenDesc.RefreshRate.Denominator = 1;
	dxgiSwapChainFullScreenDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	dxgiSwapChainFullScreenDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	dxgiSwapChainFullScreenDesc.Windowed = true;

	HRESULT hResult = m_pdxgiFactory6->CreateSwapChainForHwnd(m_pd3dCommandQueue, m_hWnd, &dxgiSwapChainDesc, &dxgiSwapChainFullScreenDesc, NULL, (IDXGISwapChain1 **)&m_pdxgiSwapChain);
#else
	DXGI_SWAP_CHAIN_DESC1 dxgiSwapChainDesc;
	::ZeroMemory(&dxgiSwapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC1));
	dxgiSwapChainDesc.Width = m_nWndClientWidth;
	dxgiSwapChainDesc.Height = m_nWndClientHeight;
	dxgiSwapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiSwapChainDesc.SampleDesc.Count = (m_bMsaa4xEnable) ? 4 : 1;
	dxgiSwapChainDesc.SampleDesc.Quality = (m_bMsaa4xEnable) ? (m_nMsaa4xQualityLevels - 1) : 0;
	dxgiSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	dxgiSwapChainDesc.BufferCount = m_nSwapChainBuffers;
	dxgiSwapChainDesc.Scaling = DXGI_SCALING_NONE;
	dxgiSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	dxgiSwapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	dxgiSwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	//dxgiSwapChainDesc.Flags = 

	//DXGI_SWAP_CHAIN_FULLSCREEN_DESC dxgiSwapChainFullScreenDesc;
	//::ZeroMemory(&dxgiSwapChainFullScreenDesc, sizeof(DXGI_SWAP_CHAIN_FULLSCREEN_DESC));
	//dxgiSwapChainFullScreenDesc.RefreshRate.Numerator = 60;
	//dxgiSwapChainFullScreenDesc.RefreshRate.Denominator = 1;
	//dxgiSwapChainFullScreenDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	//dxgiSwapChainFullScreenDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	//dxgiSwapChainFullScreenDesc.Windowed = true;

	HRESULT hResult = m_pdxgiFactory->CreateSwapChainForHwnd(m_pd3dCommandQueue, m_hWnd, &dxgiSwapChainDesc, NULL, NULL, (IDXGISwapChain1 **)&m_pdxgiSwapChain);
	/*DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
	::ZeroMemory(&dxgiSwapChainDesc, sizeof(dxgiSwapChainDesc));
	dxgiSwapChainDesc.BufferDesc.Width = m_nWndClientWidth;
	dxgiSwapChainDesc.BufferDesc.Height = m_nWndClientHeight;
	dxgiSwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiSwapChainDesc.SampleDesc.Count = (m_bMsaa4xEnable) ? 4 : 1;
	dxgiSwapChainDesc.SampleDesc.Quality = (m_bMsaa4xEnable) ? (m_nMsaa4xQualityLevels - 1) : 0;
	dxgiSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	dxgiSwapChainDesc.BufferCount = m_nSwapChainBuffers;

	dxgiSwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;

	dxgiSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	dxgiSwapChainDesc.OutputWindow = m_hWnd;
	dxgiSwapChainDesc.Windowed = true;

	dxgiSwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	HRESULT hResult = m_pdxgiFactory->CreateSwapChain(m_pd3dCommandQueue, &dxgiSwapChainDesc, (IDXGISwapChain**)&m_pdxgiSwapChain);*/
#endif	
#ifdef FullScreenMode
	hResult = m_pdxgiSwapChain->GetFullscreenState(false, NULL);
	if (FAILED(hResult))
	{
		//cout << "GetFullScreenState 에러\n";
	}
	//전체 모드로 시작
	hResult = m_pdxgiSwapChain->SetFullscreenState(true, NULL);

	if (hResult == DXGI_ERROR_NOT_CURRENTLY_AVAILABLE)
	{
		//cout << " A resource is not available at the time of the call, but may become available later.\n";
		return;
	}
	if (hResult == DXGI_STATUS_NO_REDIRECTION)
	{
		//cout << "The driver is requesting that the DXGI runtime not use shared resources to communicate with the Desktop Window Manager\n";
		return;
	}
	if (hResult == DXGI_STATUS_GRAPHICS_VIDPN_SOURCE_IN_USE)
	{
		//cout << "The Present operation was not visible because the target monitor was being used for some other purpose.\n";
		return;
	}
	if (hResult == DXGI_STATUS_MODE_CHANGED)
	{
		//cout << "The Present operation was not visible because the display mode changed. DXGI will have re-attempted the presentation.\n";
		return;
	}
	if (hResult == DXGI_STATUS_MODE_CHANGE_IN_PROGRESS)
	{
		//cout << "The Present operation was not visible because another Direct3D device was attempting to take fullscreen mode at the time.\n";
		return;
	}
	if (hResult == DXGI_STATUS_NO_DESKTOP_ACCESS)
	{
		//cout << "The Present operation was not visible because the Windows session has switched to another desktop (for example, ctrl-alt-del).\n";
		return;
	}
	if (hResult == DXGI_STATUS_OCCLUDED)
	{
	//	cout << "The Present operation was invisible to the user\n";
		return;
	}
	if (FAILED(hResult)) 
	{
	//	cout << "SetFullScreenState ERROR\n";
		return;
	}
	if (SUCCEEDED(hResult))
	{
	
		DXGI_MODE_DESC dxgiTargetParameters;
		dxgiTargetParameters.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		dxgiTargetParameters.Width = m_nWndClientWidth;
		dxgiTargetParameters.Height = m_nWndClientHeight;
		dxgiTargetParameters.RefreshRate.Numerator = 60;
		dxgiTargetParameters.RefreshRate.Denominator = 1;
		dxgiTargetParameters.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		dxgiTargetParameters.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		m_pdxgiSwapChain->ResizeTarget(&dxgiTargetParameters);
		for (int i = 0; i < m_nSwapChainBuffers; i++)
		{
			if (m_ppd3dSwapChainBackBuffers[i]) m_ppd3dSwapChainBackBuffers[i]->Release();

		}

		DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
		m_pdxgiSwapChain->GetDesc(&dxgiSwapChainDesc);
		m_pdxgiSwapChain->ResizeBuffers(m_nSwapChainBuffers, m_nWndClientWidth,m_nWndClientHeight,dxgiSwapChainDesc.BufferDesc.Format,dxgiSwapChainDesc.Flags);

	}
#else
	hResult = m_pdxgiSwapChain->SetFullscreenState(false, NULL);
#endif
	
	// 스왑체인의 현재 후면버퍼 인덱스를 저장
	m_nSwapChainBufferIndex = m_pdxgiSwapChain->GetCurrentBackBufferIndex();
	// Alt + Enter키로 전체모드 비활성화
	hResult = m_pdxgiFactory6->MakeWindowAssociation(m_hWnd, DXGI_MWA_NO_ALT_ENTER);
	//DXGI_MWA_NO_WINDOW_CHANGES -> DXGI가 윈도우 메세지큐를 감시하지 않는 옵션
	if (FAILED(hResult))
	{
		//cout << "MakeWindowAssociation 에러\n";
		return;
	}

//#ifndef _WITH_SWAPCHAIN_FULLSCREEN_STATE
	CreateRenderTargetViews();

	//ChangeSwapChainState();
//#endif
}

void CGameFramework::CreateDirect3DDevice()
{
	HRESULT hResult;

	UINT nDXGIFactoryFlags = 0;
#if defined(_DEBUG)
	ID3D12Debug *pd3dDebugController = NULL;
	hResult = D3D12GetDebugInterface(__uuidof(ID3D12Debug), (void **)&pd3dDebugController);
	if (pd3dDebugController)
	{
		pd3dDebugController->EnableDebugLayer();
		pd3dDebugController->Release();
	}
	nDXGIFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif

	hResult = ::CreateDXGIFactory2(nDXGIFactoryFlags, __uuidof(IDXGIFactory6), (void **)&m_pdxgiFactory6);
	if (FAILED(hResult))
	{
		printf("CreateDXGIFactory2 Error\n");
		return;
	}

	IDXGIAdapter1* pd3dAdapter = NULL;
	IDXGIOutput* pOutput = NULL;
	UINT nModes = 0;
	UINT nFlags = DXGI_ENUM_MODES_SCALING;
	//DXGIFactory6::EnumAdapterByGpuPreference -> This method allows developers to select which GPU they think is most appropriate for each device their app creates and utilizes.
	for (UINT i = 0; DXGI_ERROR_NOT_FOUND != m_pdxgiFactory6->EnumAdapterByGpuPreference(i,DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,IID_PPV_ARGS(&pd3dAdapter))/*m_pdxgiFactory->EnumAdapters1(i, &pd3dAdapter)*/; i++)
	{
		DXGI_ADAPTER_DESC1 dxgiAdapterDesc;
		/*pd3dAdapter->EnumOutputs(0, &pOutput);
		pOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, nFlags, &nModes, NULL);
		m_pDisplayMode = new DXGI_MODE_DESC[nModes];*/

		pd3dAdapter->GetDesc1(&dxgiAdapterDesc);
		
		//pOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, 0, &nModes, &m_pDisplayMode[0]);

		if (dxgiAdapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;
		if (SUCCEEDED(D3D12CreateDevice(pd3dAdapter, D3D_FEATURE_LEVEL_12_0, _uuidof(ID3D12Device), (void **)&m_pd3dDevice))) break;
	}

	// DirectX12를 지원하지 않는 경우
	if (pd3dAdapter == nullptr)
	{
		hResult = m_pdxgiFactory6->EnumWarpAdapter(_uuidof(IDXGIAdapter1), (void **)&pd3dAdapter);
		//if (FAILED(hResult))
			//cout << "EnumWarpAdater 에러\n";
		hResult = D3D12CreateDevice(pd3dAdapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), (void **)&m_pd3dDevice);
		//if (FAILED(hResult))
		//	cout << "CreateWarpDevice 에러\n";
	}

	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS d3dMsaaQualityLevels;
	d3dMsaaQualityLevels.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3dMsaaQualityLevels.SampleCount = 4;
	d3dMsaaQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	d3dMsaaQualityLevels.NumQualityLevels = 0;
	hResult = m_pd3dDevice->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &d3dMsaaQualityLevels, sizeof(D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS));
	m_nMsaa4xQualityLevels = d3dMsaaQualityLevels.NumQualityLevels;
	m_bMsaa4xEnable = (m_nMsaa4xQualityLevels > 1) ? true : false;

	hResult = m_pd3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, __uuidof(ID3D12Fence), (void **)&m_pd3dFence);
	//if (FAILED(hResult))
	//	cout << "CreateFence 에러\n";
	for (UINT i = 0; i < m_nSwapChainBuffers; i++) m_nFenceValues[i] = 0;

	m_hFenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);

	if (pd3dAdapter) pd3dAdapter->Release();
	if (pOutput) pOutput->Release();
	// IncrementSize를 컴퓨터에 맞게 설정해줌
	::gnCbvSrvDescriptorIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

void CGameFramework::CreateCommandQueueAndList()
{
	HRESULT hResult;

	D3D12_COMMAND_QUEUE_DESC d3dCommandQueueDesc;
	::ZeroMemory(&d3dCommandQueueDesc, sizeof(D3D12_COMMAND_QUEUE_DESC));
	d3dCommandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	d3dCommandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	hResult = m_pd3dDevice->CreateCommandQueue(&d3dCommandQueueDesc, _uuidof(ID3D12CommandQueue), (void **)&m_pd3dCommandQueue);
	//if (FAILED(hResult))
		//cout << "CreateCommandQueue 에러\n";

	hResult = m_pd3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator), (void **)&m_pd3dCommandAllocator);
	

	hResult = m_pd3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pd3dCommandAllocator, NULL, __uuidof(ID3D12GraphicsCommandList), (void **)&m_pd3dCommandList);
	
	hResult = m_pd3dCommandList->Close();
}

void CGameFramework::CreateLoadingCommandList()
{
	HRESULT Result;

	Result = m_pd3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator), (void **)&m_pLoadingCommandAllocator);
	Result = m_pd3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pLoadingCommandAllocator, nullptr, __uuidof(ID3D12GraphicsCommandList), (void **)&m_pLoadingCommandList);
	Result = m_pLoadingCommandList->Close();
}

#ifdef _WITH_DIRECT2D_
void CGameFramework::CreateDirect2DDevice()
{
	UINT nD3D11DeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#if defined(_DEBUG)
	nD3D11DeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	//ID3D12Device 생성 후 ID3D11On12Device 를 생성한다.
	//ID3D11Device는 D3D11OnCreateDevice API를 통해 ID3D12Device에 Wrapping되어있다.
	//이 api는 11On12 디바이스에 명령들을  ID3D12CommandQueue에 제출하는 역할을 한다.
	//ID3D11Device 가 생성된 후에, ID3D11On12Device 인터페이스를 ID3D11Device로 부터 요청할 수 있다.
	//ID3D11On12Device 가 D2D를 설치하는데 사용하는 주 디바이스가 된다.
	ID3D11Device *pd3d11Device = NULL;
	HRESULT hResult = ::D3D11On12CreateDevice(m_pd3dDevice, nD3D11DeviceFlags, NULL, 0, (IUnknown **)&m_pd3dCommandQueue, 1, 0, &pd3d11Device, &m_pd3d11DeviceContext, NULL);
	hResult = pd3d11Device->QueryInterface(__uuidof(ID3D11On12Device), (void **)&m_pd3d11On12Device);
	if (pd3d11Device) pd3d11Device->Release();

	//11On12 디바이스가 생성된 후, 우리는 이 디바이스를 D2DFactory와 디바이스를 
	//다이렉트 11에서 했던 것 과 동일하게 생성해준다.

	D2D1_FACTORY_OPTIONS nD2DFactoryOptions = { D2D1_DEBUG_LEVEL_NONE };
#if defined(_DEBUG)
	nD2DFactoryOptions.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
#endif

	//D2D1_FACTORY_TYPE_SINGLE_THREADED ->팩토리에 접근하거나 쓰는것 또는 객체를 생성하는 것으로부터 동기화를 제공하지 않는다.
	//									만약 팩토리나 객체들이 다중 스레드에서 호출되면, Application은 접근을 lock해준다.
	hResult = ::D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory3), &nD2DFactoryOptions, (void**)&m_pd2dFactory);

	////////////////////////////////////////////////////////////////////////////////////
	IDXGIDevice *pdxgiDevice = NULL;
	hResult = m_pd3d11On12Device->QueryInterface(__uuidof(IDXGIDevice), (void **)&pdxgiDevice);
	hResult = m_pd2dFactory->CreateDevice(pdxgiDevice, &m_pd2dDevice);
	hResult = m_pd2dDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &m_pd2dDeviceContext);
	hResult = ::DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory5), (IUnknown **)&m_pdWriteFactory);

	if (pdxgiDevice)
		pdxgiDevice->Release();

	m_pd2dDeviceContext->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);

	// Bitmap 이미지를 생성한다.
	Initialize_BitmapImage();
	// 게임에 필요한 폰트를 생성한다.
	Initialize_GameFont();
}

void CGameFramework::Initialize_BitmapImage()
{
	CoInitialize(NULL);
	HRESULT hResult = CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, __uuidof(IWICImagingFactory), (void**)&m_pwicImagingFactory);

	wstring imagePath[] = 
	{
		L"../Resource/Png/Characters.png",
		L"../Resource/Png/ChoiceCharacter.png",
		L"../Resource/Png/ScoreBoard.png",
		L"../Resource/Png/TimeOver.png", 
		L"../Resource/Png/Host.png",
		L"../Resource/Png/IPNotice.png",
		L"../Resource/Png/ChatInput.png"
	};

	UINT originX = 1280;
	UINT originY = 720;

	// 스코어 보드 이미지 위치
	RECT r;
	GetClientRect(m_hWnd, &r);
	r.left = r.right / 5;
	r.top = r.bottom / 5;
	r.right = r.right * 4 / 5;
	r.bottom = r.bottom * 4 / 5;

	D2D1_RECT_F charactersPos = D2D1::RectF((781 * FRAME_BUFFER_WIDTH) / originX, (17 * FRAME_BUFFER_HEIGHT) / originY, (1262 * FRAME_BUFFER_WIDTH) / originX, (412 * FRAME_BUFFER_HEIGHT) / originY);
	D2D1_RECT_F choicePos = D2D1::RectF((90 * FRAME_BUFFER_WIDTH) / originX, (66 * FRAME_BUFFER_HEIGHT) / originY, (210 * FRAME_BUFFER_WIDTH) / originX, (186 * FRAME_BUFFER_HEIGHT) / originY);
	D2D1_RECT_F scoreboardPos = D2D1::RectF(r.left, r.top, r.right, r.bottom);
	D2D1_RECT_F timeoverPos = D2D1::RectF(scoreboardPos.left, 80, scoreboardPos.right, 230);
	D2D1_RECT_F hostPos = D2D1::RectF(scoreboardPos.left, 80, scoreboardPos.right, 230);
	D2D1_RECT_F chatinputPos = D2D1::RectF((15.0f * FRAME_BUFFER_WIDTH) / originX, (675.0f * FRAME_BUFFER_HEIGHT) / originY, (705.0f * FRAME_BUFFER_WIDTH) / originX, (715.0f * FRAME_BUFFER_HEIGHT) / originY);

	CDirect2D::GetInstance()->CreateBitmapImage(ImageInfo(imagePath[0], charactersPos, 4900, 575, 7, 1, 0, 0), "Characters");
	CDirect2D::GetInstance()->CreateBitmapImage(ImageInfo(imagePath[1], choicePos, 1800, 300, 6, 1, 0, 0), "ChoiceCharacter");
	CDirect2D::GetInstance()->CreateBitmapImage(ImageInfo(imagePath[2], scoreboardPos, 1638, 1009, 1, 1, 0, 0), "ScoreBoard");
	CDirect2D::GetInstance()->CreateBitmapImage(ImageInfo(imagePath[3], timeoverPos, 2138, 569, 1, 1, 0, 0), "TimeOver");
	CDirect2D::GetInstance()->CreateBitmapImage(ImageInfo(imagePath[4], hostPos, 308, 300, 1, 1, 0, 0), "Host");
	CDirect2D::GetInstance()->CreateBitmapImage(ImageInfo(imagePath[5], hostPos, 1210, 250, 2, 1, 0, 0), "IPNotice");
	CDirect2D::GetInstance()->CreateBitmapImage(ImageInfo(imagePath[6], chatinputPos, 574, 28, 1, 1, 0, 0), "ChatInput");
}

void CGameFramework::Initialize_GameFont()
{
	CDirect2D::GetInstance()->CreateGameFont();
	CDirect2D::GetInstance()->CreateGameFontColor();

	ChattingSystem::GetInstance()->Initialize();
	//ChattingSystem::GetInstance()->Initialize(CDirect2D::GetInstance()->GetFontInfo("메이플").m_pFont, CDirect2D::GetInstance()->GetFontInfo("메이플").m_pTextLayout, CDirect2D::GetInstance()->GetFontColor("검은색"), m_pwicImagingFactory, m_pd2dDeviceContext);
	//ChattingSystem::GetInstance()->SetFontSize(CDirect2D::GetInstance()->GetFontInfo("메이플").m_FontSize);
	//ChattingSystem::GetInstance()->CreateChattingFont();
}

#endif

void CGameFramework::CreateRtvAndDsvDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	::ZeroMemory(&d3dDescriptorHeapDesc, sizeof(D3D12_DESCRIPTOR_HEAP_DESC));
	d3dDescriptorHeapDesc.NumDescriptors = m_nSwapChainBuffers;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	d3dDescriptorHeapDesc.NodeMask = 0;
	HRESULT hResult = m_pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void **)&m_pd3dRtvDescriptorHeap);
	m_nRtvDescriptorIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	d3dDescriptorHeapDesc.NumDescriptors = 1;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	hResult = m_pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void **)&m_pd3dDsvDescriptorHeap);
	m_nDsvDescriptorIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
}

void CGameFramework::CreateRenderTargetViews()
{
	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = m_pd3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	for (UINT i = 0; i < m_nSwapChainBuffers; i++)
	{
		m_pdxgiSwapChain->GetBuffer(i, __uuidof(ID3D12Resource), (void **)&m_ppd3dSwapChainBackBuffers[i]);
		m_pd3dDevice->CreateRenderTargetView(m_ppd3dSwapChainBackBuffers[i], NULL, d3dRtvCPUDescriptorHandle);
		d3dRtvCPUDescriptorHandle.ptr += m_nRtvDescriptorIncrementSize;
	}
#ifdef _WITH_DIRECT2D_
	CreateDirect2DRenderTargetViews();	
#endif
}

#ifdef _WITH_DIRECT2D_
// D3D12가 스왑체인을 소유한다. 그렇기 때문에 우리가 사용하는 11On12 Device에 백버퍼에 우리가 원하는것을 그리려면 
// 우리는 ID3D12Resource 의 BackBuffer 로 부터 Wrapped Resource of type ID3D11Resource.
// 이것은 ID3D12Resource 와 D3D11 기반의 인터페이스들을 연결해준다.
// 이것은 또한 11On12Device에서도 사용가능하다. 
// 우리는 Wrapped Resource 를 가진 후 부터 우리는 D2D Render Target Surface 를 생성할 수 있다.
// 뿐만 아니라 Load Assets메소드안 에 있는 렌더 타겟도 가능하다.
void CGameFramework::CreateDirect2DRenderTargetViews()
{
	float fxDPI, fyDPI;

	m_pd2dFactory->GetDesktopDpi(&fxDPI, &fyDPI); // 현재 데스크톱에 dots per inch(DPI)를 검색하고 ,각 변수들에 할당해준다.
	
	//D2D1_BITMAP_PROPERTIES1	 : 
	//1. D2D1_BITMAP_OPTIONS     : 비트맵의 용도 옵션.  EX> D2D1_BITMAP_OPTIONS_TARGET : the bitmap can be used as a device context target.
	//												   EX> D2D1_BITMAP_OPTIONS_CANNOT_DRAW : the bitmap cannot be used an input.
	//2. const D2D1_PIXEL_FORMAT : 비트맵의 픽셀 형식과 알파 모드.  EX> D2D1_ALPHA_MODE_PREMULTIPLIED :  알파값이 미리 곱해진다.
	D2D1_BITMAP_PROPERTIES1 d2dBitmapProperties = D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET  |  D2D1_BITMAP_OPTIONS_CANNOT_DRAW , D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED), fxDPI, fxDPI);

	for (UINT i = 0; i < m_nSwapChainBuffers; i++)
	{
		D3D11_RESOURCE_FLAGS d3d11Flags = { D3D11_BIND_RENDER_TARGET };
		//CreateWrappedResource-> 이 함수는 D3D11On12 에서 사용가능한  D3D11 리소스들을 만들어준다.
		m_pd3d11On12Device->CreateWrappedResource(m_ppd3dSwapChainBackBuffers[i], &d3d11Flags, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT, __uuidof(ID3D11Resource), (void**)&m_ppd3d11WrappedBackBuffers[i]);
		

		IDXGISurface *pdxgiSurface = NULL;
		m_ppd3d11WrappedBackBuffers[i]->QueryInterface(__uuidof(IDXGISurface), (void**)&pdxgiSurface);

		//CreateBitmapFromDxgiSurface() -> DXGI표면에서 대상 표면으로 설정하거나 추가 색상 컨텍스트 정보를 지정할 수 있는 비트맵을 만듭니다. 
		m_pd2dDeviceContext->CreateBitmapFromDxgiSurface(pdxgiSurface, &d2dBitmapProperties, &m_ppd2dRenderTargets[i]);
		if (pdxgiSurface)
		{
			pdxgiSurface->Release();
		}
	}
}
#endif

void CGameFramework::CreateDepthStencilView()
{
	D3D12_RESOURCE_DESC d3dResourceDesc;
	d3dResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	d3dResourceDesc.Alignment = 0;
	d3dResourceDesc.Width = m_nWndClientWidth;
	d3dResourceDesc.Height = m_nWndClientHeight;
	d3dResourceDesc.DepthOrArraySize = 1;
	d3dResourceDesc.MipLevels = 1;
	d3dResourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dResourceDesc.SampleDesc.Count = (m_bMsaa4xEnable) ? 4 : 1;
	d3dResourceDesc.SampleDesc.Quality = (m_bMsaa4xEnable) ? (m_nMsaa4xQualityLevels - 1) : 0;
	d3dResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	d3dResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_HEAP_PROPERTIES d3dHeapProperties;
	::ZeroMemory(&d3dHeapProperties, sizeof(D3D12_HEAP_PROPERTIES));
	d3dHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	d3dHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	d3dHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	d3dHeapProperties.CreationNodeMask = 1;
	d3dHeapProperties.VisibleNodeMask = 1;

	D3D12_CLEAR_VALUE d3dClearValue;
	d3dClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dClearValue.DepthStencil.Depth = 1.0f;
	d3dClearValue.DepthStencil.Stencil = 0;

	m_pd3dDevice->CreateCommittedResource(&d3dHeapProperties, D3D12_HEAP_FLAG_NONE, &d3dResourceDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &d3dClearValue, __uuidof(ID3D12Resource), (void **)&m_pd3dDepthStencilBuffer);

	D3D12_DEPTH_STENCIL_VIEW_DESC d3dDepthStencilViewDesc;
	::ZeroMemory(&d3dDepthStencilViewDesc, sizeof(D3D12_DEPTH_STENCIL_VIEW_DESC));
	d3dDepthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dDepthStencilViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	d3dDepthStencilViewDesc.Flags = D3D12_DSV_FLAG_NONE;

	D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle = m_pd3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_pd3dDevice->CreateDepthStencilView(m_pd3dDepthStencilBuffer, &d3dDepthStencilViewDesc, d3dDsvCPUDescriptorHandle);
}

void CGameFramework::ChangeSwapChainState()
{
	WaitForGpuComplete();

	BOOL bFullScreenState = FALSE;
	HRESULT hResult = m_pdxgiSwapChain->GetFullscreenState(&bFullScreenState, NULL);
	if (FAILED(hResult))
	{
		//cout << "GetFullScreenState 에러\n";
		return;
	}
		
	hResult = m_pdxgiSwapChain->SetFullscreenState(!bFullScreenState, NULL);
	if (FAILED(hResult))
	{
		//cout << "SetFullScreenState 에러\n";
		return;
	}
	DXGI_MODE_DESC dxgiTargetParameters;
	dxgiTargetParameters.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiTargetParameters.Width = m_nWndClientWidth;
	dxgiTargetParameters.Height = m_nWndClientHeight;
	dxgiTargetParameters.RefreshRate.Numerator = 60;
	dxgiTargetParameters.RefreshRate.Denominator = 1;
	dxgiTargetParameters.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	dxgiTargetParameters.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	m_pdxgiSwapChain->ResizeTarget(&dxgiTargetParameters);

	for (int i = 0; i < m_nSwapChainBuffers; i++)
	{
		if (m_ppd3dSwapChainBackBuffers[i]) m_ppd3dSwapChainBackBuffers[i]->Release();
#ifdef _WITH_DIRECT2D_
		if (m_ppd3d11WrappedBackBuffers[i]) m_ppd3d11WrappedBackBuffers[i]->Release();
		if (m_ppd2dRenderTargets[i]) m_ppd2dRenderTargets[i]->Release();
#endif
	}
	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
	m_pdxgiSwapChain->GetDesc(&dxgiSwapChainDesc);
	m_pdxgiSwapChain->ResizeBuffers(m_nSwapChainBuffers, m_nWndClientWidth, m_nWndClientHeight, dxgiSwapChainDesc.BufferDesc.Format, dxgiSwapChainDesc.Flags);

	m_nSwapChainBufferIndex = m_pdxgiSwapChain->GetCurrentBackBufferIndex();

	CreateRenderTargetViews();
}

void CGameFramework::CreateOffScreenRenderTargetViews()
{
	CTexture *pTextureForCartoonProcessing = new CTexture(m_nCartoonScreenRenderTargetBuffers, RESOURCE_TEXTURE2D_ARRAY, 0);

	D3D12_CLEAR_VALUE d3dClearValue = { DXGI_FORMAT_R8G8B8A8_UNORM, { 0.0f, 0.0f, 0.0f, 1.0f } };

	for (UINT i = 0; i < m_nCartoonScreenRenderTargetBuffers; i++)
	{
		m_ppd3dCartoonScreenRenderTargetBuffers[i] = pTextureForCartoonProcessing->CreateTexture(m_pd3dDevice, m_pd3dCommandList, m_nWndClientWidth, m_nWndClientHeight, DXGI_FORMAT_R8G8B8A8_UNORM, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, i);
		m_ppd3dCartoonScreenRenderTargetBuffers[i]->AddRef();
	}

	//RtvCPU서술자 증가 크기가 스왑체인의 크기 배수로 증가하는 이유?
	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = m_pd3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	d3dRtvCPUDescriptorHandle.ptr += (m_nSwapChainBuffers * m_nRtvDescriptorIncrementSize);

	D3D12_RENDER_TARGET_VIEW_DESC d3dRenderTargetViewDesc;
	d3dRenderTargetViewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3dRenderTargetViewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	d3dRenderTargetViewDesc.Texture2D.MipSlice = 0;
	d3dRenderTargetViewDesc.Texture2D.PlaneSlice = 0;

	m_pCartoonShader = new CSobelCartoonShader();
	m_pCartoonShader->CreateGraphicsRootSignature(m_pd3dDevice);
	m_pCartoonShader->CreateShader(m_pd3dDevice, m_pCartoonShader->GetGraphicsRootSignature(), 1);
	m_pCartoonShader->BuildObjects(m_pd3dDevice, m_pd3dCommandList, pTextureForCartoonProcessing);

}

void CGameFramework::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (g_State)
	{
	case CONNECT:
		{
			if(m_pIPScene)
				m_pIPScene->OnProcessingMouseMessage(hWnd, nMessageID, wParam, lParam);
		break;
		}
		case LOBBY:
		{
			if (m_pLobbyScene)
				m_pLobbyScene->OnProcessingMouseMessage(hWnd, nMessageID, wParam, lParam);
			break;
		}

		case INGAME:
		{
			if (m_pScene)
				m_pScene->OnProcessingMouseMessage(hWnd, nMessageID, wParam, lParam);
			break;
		}

#ifdef _WITH_SERVER_
		case LOGIN:
		{
			if (m_pLoginScene)
			{
				int ret = m_pLoginScene->OnProcessingMouseMessage(hWnd, nMessageID, wParam, lParam);

				if(ret == CIDShader::state::REQUEST_LOGIN)		
					Network::GetInstance()->SendNickName(m_pPlayer->GetPlayerID(), m_pLoginScene->GetIDInstance()->GetPlayerName());
	
				if (m_pLoginScene->IsLogin())
				{
					g_State = LOBBY;
					m_pPlayer->SetPlayerName(m_pLoginScene->GetIDInstance()->GetPlayerName());
				}
			}
			break;
		}

#endif
		default:
			break;
	}
	

	switch (nMessageID)
	{
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
		::SetCapture(hWnd);
	
		::GetCursorPos(&m_ptOldCursorPos);
		break;
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
		::ReleaseCapture();
		break;
	case WM_MOUSEMOVE:
		break;
	default:
		break;
	}
}

void CGameFramework::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (g_State)
	{
		case INGAME:
		{
			if (m_pScene)
			{
				if(ChattingSystem::GetInstance()->IsChattingActive()==false)
					m_pScene->OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);
#ifndef _WITH_SERVER_

				if (wParam == '0')
				{
					g_Round = STAGE::ROUND_1;
					CSoundSystem::StopSound(CSoundSystem::SOUND_TYPE::FIRE_SOUND);
					m_pScene->ChangeRound();
				}

				else if (wParam == '1')
				{
					g_Round = STAGE::ROUND_2;
					CSoundSystem::StopSound(CSoundSystem::SOUND_TYPE::FIRE_SOUND);
					m_pScene->ChangeRound();
				}

				else if (wParam == '2')
				{
					g_Round = STAGE::ROUND_3;
					CSoundSystem::PlayingSound(CSoundSystem::SOUND_TYPE::FIRE_SOUND, 0.5f);
					m_pScene->ChangeRound();
				}
#endif
			}	
			break;
		}

		case LOBBY:
		{
			if (m_pLobbyScene)
				m_pLobbyScene->OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);
			break;
		}

		default:
			break;
	}
	
	switch (nMessageID)
	{
	case WM_KEYUP:
		switch (wParam)
		{
		case VK_F1:
		case VK_F2:
		case VK_F3:
			m_pCamera = m_pPlayer->ChangeCamera((DWORD)(wParam - VK_F1 + 1), m_GameTimer.GetTimeElapsed());
			break;

#ifdef _WITH_SERVER_
		case VK_UP:
		case VK_DOWN:
		{
			Network::GetInstance()->SendReleaseKey();
			break;
		}
#endif

		/*case VK_F9:
			ChangeSwapChainState();
			break;*/
#ifdef _WITH_DIRECT2D_
		case VK_RETURN:
		{
			if (g_State == INGAME)
			{
				(ChattingSystem::GetInstance()->IsChattingActive()) ? ChattingSystem::GetInstance()->SetActive(false)
					: ChattingSystem::GetInstance()->SetActive(true);
			}
			if (g_State == LOBBY)
			{
				(ChattingSystem::GetInstance()->IsChattingActive()) ? ChattingSystem::GetInstance()->SetActive(false)
					: ChattingSystem::GetInstance()->SetActive(true);
			}
			break;
		}
		case VK_HANGEUL:
			(m_bHangeul) ? m_bHangeul = false : m_bHangeul = true;
			ChattingSystem::GetInstance()->SetIMEMode(hWnd, m_bHangeul);
#ifdef _WITH_SERVER_
			if (g_State == LOGIN)
			{
				if (m_pLoginScene)
					m_pLoginScene->GetIDInstance()->SetIMEMode(hWnd, m_bHangeul);
			}
#endif
			break;
#endif

		default:
			break;
		}
		break;
	}
}

#ifdef _MAPTOOL_MODE_
void CGameFramework::OnMapToolInputMesseage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_KEYUP:
		if(m_pMapToolShader && m_pPlayer)
			m_pMapToolShader->InstallMapObject(m_pd3dDevice, m_pd3dCommandList, m_pScene->GetGraphicsRootSignature(), m_pPlayer, wParam);
		break;
		
	default:
		break;
	}
}
#endif

LRESULT CALLBACK CGameFramework::OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_ACTIVATE:
	{
		if (LOWORD(wParam) == WA_INACTIVE)
			m_GameTimer.Stop();
		else
			m_GameTimer.Start();
		break;
	}
	case WM_SIZE:
	{
		DXGI_MODE_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.Width = 800;
		desc.Height = 600;
		break;
	}
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MOUSEMOVE:
		OnProcessingMouseMessage(hWnd, nMessageID, wParam, lParam);
		break;
	case WM_KEYDOWN:
	case WM_KEYUP:
		
		OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);

#ifdef _MAPTOOL_MODE_
		OnMapToolInputMesseage(hWnd, nMessageID, wParam, lParam);
#endif
		break;
	}
	return(0);
}

void CGameFramework::OnDestroy()
{
	CSoundSystem::Release();

	ReleaseObjects();

	//if (m_pDisplayMode)
	//	delete m_pDisplayMode;
	::CloseHandle(m_hFenceEvent);
#ifdef _WITH_DIRECT2D_

	CDirect2D::GetInstance()->Release();
	CDirect2D::GetInstance()->DeleteInstance();

	ChattingSystem::GetInstance()->Destroy();
	ChattingSystem::GetInstance()->DeleteInstance();

	if (m_pwicImagingFactory) 
		m_pwicImagingFactory->Release();

	if (m_pdWriteFactory)
		m_pdWriteFactory->Release();

	//Direct11
	if (m_pd2dDeviceContext) m_pd2dDeviceContext->Release();
	if (m_pd2dDevice) m_pd2dDevice->Release();
	if (m_pd3d11On12Device) m_pd3d11On12Device->Release();
	if (m_pd3d11DeviceContext) m_pd3d11DeviceContext->Release();
	if (m_pd2dFactory) m_pd2dFactory->Release();

	for (int i = 0; i < m_nSwapChainBuffers; i++)
	{
		if (m_ppd3d11WrappedBackBuffers[i]) m_ppd3d11WrappedBackBuffers[i]->Release();
		if (m_ppd2dRenderTargets[i]) m_ppd2dRenderTargets[i]->Release();
	}
#endif
	if (m_pd3dDepthStencilBuffer) m_pd3dDepthStencilBuffer->Release();
	if (m_pd3dDsvDescriptorHeap) m_pd3dDsvDescriptorHeap->Release();

	for (int i = 0; i < m_nSwapChainBuffers; i++) if (m_ppd3dSwapChainBackBuffers[i]) m_ppd3dSwapChainBackBuffers[i]->Release();
	if (m_pd3dRtvDescriptorHeap) m_pd3dRtvDescriptorHeap->Release();

	if (m_pd3dCommandAllocator)
		m_pd3dCommandAllocator->Release();
	if (m_pd3dCommandQueue)
		m_pd3dCommandQueue->Release();
	if (m_pd3dCommandList)
		m_pd3dCommandList->Release();

	if (m_pLoadingCommandAllocator)
		m_pLoadingCommandAllocator->Release();
	if (m_pLoadingCommandList)
		m_pLoadingCommandList->Release();

	if (m_pd3dFence) m_pd3dFence->Release();

	m_pdxgiSwapChain->SetFullscreenState(FALSE, NULL);
	if (m_pdxgiSwapChain) m_pdxgiSwapChain->Release();
	if (m_pd3dDevice) m_pd3dDevice->Release();
	//if (m_pdxgiFactory) m_pdxgiFactory->Release();

	if (m_pdxgiFactory6) m_pdxgiFactory6->Release();
#if defined(_DEBUG)
	IDXGIDebug1	*pdxgiDebug = NULL;
	DXGIGetDebugInterface1(0, __uuidof(IDXGIDebug1), (void **)&pdxgiDebug);
	HRESULT hResult = pdxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_DETAIL);
	pdxgiDebug->Release();
#endif
}

bool CGameFramework::BuildObjects()
{
	m_pd3dCommandList->Reset(m_pd3dCommandAllocator, NULL);

#ifdef _WITH_SERVER_
	m_pIPScene = new CIPScene;
	m_pIPScene->BuildObjects(m_pd3dDevice, m_pd3dCommandList, CDirect2D::GetInstance()->GetFontInfo("메이플").m_pFont, CDirect2D::GetInstance()->GetFontInfo("메이플").m_pTextLayout, CDirect2D::GetInstance()->GetFontColor("검은색"), m_pd2dDeviceContext);
	Network::GetInstance()->SetGameFrameworkPtr(m_hWnd, this);

#endif
	// 윈도우 창 띄우기
	g_State = LOADING;
	loadingThread.emplace_back(thread{ &CGameFramework::Worker_Thread, this });

#ifdef _WITH_SERVER_
	m_pLoginScene = new CLoginScene;
	if (m_pLoginScene)
		m_pLoginScene->BuildObjects(m_pd3dDevice, m_pd3dCommandList, CDirect2D::GetInstance()->GetFontInfo("메이플").m_pFont, CDirect2D::GetInstance()->GetFontInfo("메이플").m_pTextLayout, CDirect2D::GetInstance()->GetFontColor("검은색"));
#endif

	// 사운드 로딩
	CResourceManager::PrepareSound();

	m_pLobbyScene = new CLobbyScene;
	if (!m_pLobbyScene)
		return false;

	m_pLobbyScene->BuildObjects(m_pd3dDevice, m_pd3dCommandList);

	const int nPlayerCount = 6;		//임시로 플레이어 개수 지정. 
	//추후에 서버에서 접속 인원을 씬 BuildObject 호출 전에 받아와서 세팅하면 될듯함.
	m_pScene = new CScene;
	if (m_pScene)
	{
		//GameFramework에서 관리하는 CPlayer를 제외한 나머지 넘겨준다.
		m_pScene->BuildObjects(m_pd3dDevice, m_pd3dCommandList, nPlayerCount);

		//g_State = INGAME;
	}
	CTerrainPlayer* pPlayer{ nullptr };

	if (m_pScene && m_pScene->getShaderManager())
	{
		map<string, CShader*> m = m_pScene->getShaderManager()->getShaderMap();
		auto iter = m.find("Terrain");
		if (iter != m.end())
		{
			CTerrain* pTerrain = dynamic_cast<CTerrainShader*>((*iter).second)->getTerrain();
			pPlayer = new CTerrainPlayer(m_pd3dDevice, m_pd3dCommandList, m_pScene->GetGraphicsRootSignature(), g_PlayerCharacter, pTerrain);
			pPlayer->SetPosition(XMFLOAT3(40.f, 0.f, 40.f));
			
			pPlayer->SetScale(XMFLOAT3(10.0f,10.0f,10.0f));
			pPlayer->SetPlayerName(L"주인공");
			pPlayer->SetScore(0);

			map<string, Bounds*> BoundMap = m_pScene->getShaderManager()->getResourceManager()->getBoundMap();
			auto iter2 = BoundMap.find("EvilBear");
			if (iter2 != BoundMap.end()) 
			{
				pPlayer->SetOOBB((*iter2).second->m_xmf3Center, (*iter2).second->m_xmf3Extent, XMFLOAT4(0, 0, 0, 1));
			}
#ifndef _WITH_SERVER_
#ifdef _MAPTOOL_MODE_
			m_pMapToolShader = new CMapToolShader;
			m_pMapToolShader->BuildObjects(m_pScene->getShaderManager()->getResourceManager()->getModelMap());
#endif
#endif
		}
	}

	if (m_pScene)
	{
		m_pPlayer = pPlayer;
		m_pScene->setPlayer(m_pPlayer);
		m_pCamera = m_pPlayer->GetCamera();
	}

	for (auto& thread : loadingThread)
		thread.join();

	m_pd3dCommandList->Close();
	ID3D12CommandList* ppd3dCommandLists[] = { m_pd3dCommandList };
	m_pd3dCommandQueue->ExecuteCommandLists(1, ppd3dCommandLists);

	WaitForGpuComplete();
#ifdef _WITH_SERVER_
	g_State = CONNECT;
#else
	g_State = LOBBY;
#endif

	if (m_pIPScene)
		m_pIPScene->ReleaseUploadBuffers();
	if (m_pLoginScene)
		m_pLoginScene->ReleaseUploadBuffers();
	if (m_pLobbyScene)
		m_pLobbyScene->ReleaseUploadBuffers();
	if (m_pScene)
		m_pScene->ReleaseUploadBuffers();
	if (m_pPlayer)
		m_pPlayer->ReleaseUploadBuffers();

	m_GameTimer.Reset();

	return true;
}

void CGameFramework::ReleaseObjects()
{
#ifdef _MAPTOOL_MODE_
	if (m_pMapToolShader)
	{
		m_pMapToolShader->ReleaseShaderVariables();
		m_pMapToolShader->ReleaseObjects();
		delete m_pMapToolShader;
	}
#endif

	if (m_pPlayer)
		m_pPlayer->Release();

	if (m_pScene)
	{
		m_pScene->ReleaseObjects();
		delete m_pScene;
	}

	if (m_pCartoonShader)
	{
		m_pCartoonShader->ReleaseShaderVariables();
		m_pCartoonShader->ReleaseObjects();
		delete m_pCartoonShader;
	}
	if (m_pLobbyScene)
	{
		m_pLobbyScene->ReleaseObjects();
		delete m_pLobbyScene;
	}

	if (m_pLoadingScene)
	{
		m_pLoadingScene->ReleaseObjects();
		delete m_pLoadingScene;
	}
#ifdef _WITH_SERVER_
	if(m_pLoginScene)
	{
		m_pLoginScene->ReleaseObjects();
		delete m_pLoginScene;
	}
	if (m_pIPScene)
	{
		m_pIPScene->ReleaseObjects();
		delete m_pIPScene;
	}
#endif
}

void CGameFramework::ProcessInput()
{
	static UCHAR pKeysBuffer[256];
	bool bProcessedByScene = false;

	if (GetKeyboardState(pKeysBuffer) && m_pScene) bProcessedByScene = m_pScene->ProcessInput(pKeysBuffer);
	if (g_State == INGAME)
	{
		if (!bProcessedByScene)
		{
			m_pPlayer->ProcessMove();

//서버와 연동되어 있지 않을때만 마우스 회전을 허용한다.
#ifndef _WITH_SERVER_
			float cxDelta = 0.0f, cyDelta = 0.0f;
			POINT ptCursorPos;
			if (GetCapture() == m_hWnd)
			{
				SetCursor(NULL);
				GetCursorPos(&ptCursorPos);
				cxDelta = (float)(ptCursorPos.x - m_ptOldCursorPos.x) / 3.0f;
				cyDelta = (float)(ptCursorPos.y - m_ptOldCursorPos.y) / 3.0f;
				SetCursorPos(m_ptOldCursorPos.x, m_ptOldCursorPos.y);
			}

			if ((dwDirection != 0) || (cxDelta != 0.0f) || (cyDelta != 0.0f))
			{
				if (cxDelta || cyDelta)
				{
					if (pKeysBuffer[VK_RBUTTON] & 0xF0)
						m_pPlayer->Rotate(cyDelta, 0.0f, -cxDelta);
					else
						m_pPlayer->Rotate(cyDelta, cxDelta, 0.0f);
				}
				if (dwDirection) m_pPlayer->Move(dwDirection, 12.25f, true);
			}

#endif
		}
		if (g_State == INGAME) 
		{
			m_pPlayer->Update(m_GameTimer.GetTimeElapsed());
		}
	}
}

void CGameFramework::CheckIceChangeOk()
{

	using namespace std::chrono;

	if (m_IceChangeOk == true)
		return;

	m_RoundCountTime = steady_clock::now();

	seconds currentSeconds = duration_cast<seconds> (m_RoundCountTime - m_RoundStartTime);

	if(currentSeconds.count() > 5.0f)
	{
		//cout << currentSeconds.count() << endl;
		//cout << "얼음 해도됨\n";
		m_IceChangeOk = true;
	}
}
void CGameFramework::AnimateObjects()
{
	m_elapsedTime = m_GameTimer.GetTimeElapsed();
	switch(g_State)
	{
		case INGAME:
		{
			if (m_pScene)
				m_pScene->AnimateObjects(m_pd3dCommandList, m_elapsedTime);
			break;
		}

		default:
			break;
	}
}

void CGameFramework::WaitForGpuComplete()
{
	const UINT64 nFenceValue = ++m_nFenceValues[m_nSwapChainBufferIndex];
	HRESULT hResult = m_pd3dCommandQueue->Signal(m_pd3dFence, nFenceValue);

	if (m_pd3dFence->GetCompletedValue() < nFenceValue)
	{
		hResult = m_pd3dFence->SetEventOnCompletion(nFenceValue, m_hFenceEvent);
		::WaitForSingleObject(m_hFenceEvent, INFINITE);
	}
}

void CGameFramework::MoveToNextFrame()
{
	m_nSwapChainBufferIndex = m_pdxgiSwapChain->GetCurrentBackBufferIndex();
	//m_nSwapChainBufferIndex = (m_nSwapChainBufferIndex + 1) % m_nSwapChainBuffers;

	UINT64 nFenceValue = ++m_nFenceValues[m_nSwapChainBufferIndex];
	HRESULT hResult = m_pd3dCommandQueue->Signal(m_pd3dFence, nFenceValue);

	if (m_pd3dFence->GetCompletedValue() < nFenceValue)
	{
		hResult = m_pd3dFence->SetEventOnCompletion(nFenceValue, m_hFenceEvent);
		::WaitForSingleObject(m_hFenceEvent, INFINITE);
	}
}

#ifdef _WITH_DIRECT2D_
void CGameFramework::ShowIceCooltime()
{
	if(m_pPlayer)
	{
		if (m_pPlayer->GetIsShowCoolTime())
		{
			const XMFLOAT2& ndcSpace = m_pPlayer->ConvertIceCoolTimeTextToNDCSpace();

			D2D1_RECT_F iceCoolTimeText{ 0.0f,0.0f,0.0f,0.0f };
			iceCoolTimeText = D2D1::RectF(ndcSpace.x - 200.0f, ndcSpace.y, ndcSpace.x + 200.0f, ndcSpace.y);

			string coolTime = to_string(m_pPlayer->GetIceCoolTime());
			wchar_t w_coolTime[16];


			int nLen = MultiByteToWideChar(CP_ACP, 0, coolTime.c_str(), strlen(coolTime.c_str()), NULL, NULL);
			MultiByteToWideChar(CP_ACP, 0, coolTime.c_str(), strlen(coolTime.c_str()), w_coolTime, nLen);
			w_coolTime[nLen] = '\0';
			CDirect2D::GetInstance()->Render("피오피동글", "하늘색", w_coolTime, iceCoolTimeText);
		}
	}
}
void CGameFramework::SetNamecard()
{
	if (m_pScene)
	{
		map<string, CShader*> m = m_pScene->getShaderManager()->getShaderMap();
		auto iter = m.find("OtherPlayer");
		if (iter != m.end())
		{
#ifdef _WITH_SERVER_
			//vector<pair<char, char>> vec = dynamic_cast<CSkinnedAnimationObjectShader*>((*iter).second)->m_vMaterial;
			for (auto user : m_mapClients)
			{
				char id = user.second.id;
				if (id == m_pPlayer->GetPlayerID())
					continue;
				XMFLOAT2& ndcSpace = m_pScene->ProcessNameCard(id);
				D2D1_RECT_F nameCard{ 0.0f,0.0f,0.0f,0.0f };
				nameCard = D2D1::RectF(ndcSpace.x - 200.0f, ndcSpace.y, ndcSpace.x + 200.0f, ndcSpace.y);

				wchar_t name[16];
				int nLen = MultiByteToWideChar(CP_ACP, 0, m_mapClients[id].name, strlen(m_mapClients[id].name), NULL, NULL);
				MultiByteToWideChar(CP_ACP, 0, m_mapClients[id].name, strlen(m_mapClients[id].name), name, nLen);
				name[nLen] = '\0';
				CDirect2D::GetInstance()->Render("피오피동글", "갈색", name, nameCard);
			}
#else

			for (int i = 0; i < (*iter).second->m_nObjects; ++i)
			{
				XMFLOAT2& ndcSpace = m_pScene->ProcessNameCard(i);
				//	
				D2D1_RECT_F nameCard{ 0.0f,0.0f,0.0f,0.0f };
			
				nameCard = D2D1::RectF(ndcSpace.x - 60.0f, ndcSpace.y - 60.0f, ndcSpace.x + 60.0f, ndcSpace.y + 60.0f);

				CDirect2D::GetInstance()->Render("피오피동글", "빨간색", (*iter).second->m_ppObjects[i]->GetPlayerName(), nameCard);
			}
#endif
		}
	}
}


void CGameFramework::ProcessDirect2D()
{
	//AcquireWrappedResources() D3D11On12 디바이스에서 사용될 수 있는 D3D11 리소스들을 얻게해준다.
	//이 함수는 렌더링 할 Wrapped Resource 들을 다시 사용할 수 있다고 암시해준다. 
	m_pd3d11On12Device->AcquireWrappedResources(&m_ppd3d11WrappedBackBuffers[m_nSwapChainBufferIndex], 1);

	//Direct2D 디바이스가 렌더 할 비트맵이나 커맨드 리스트를 설정한다.
	m_pd2dDeviceContext->SetTarget(m_ppd2dRenderTargets[m_nSwapChainBufferIndex]);

	m_pd2dDeviceContext->BeginDraw();

	// Apply the rotation transform to the render target
	m_pd2dDeviceContext->SetTransform(D2D1::Matrix3x2F::Identity());

	//D2D1_SIZE_F : float형태의 가로 세로 쌍을 저장한 구조체
	D2D1_SIZE_F szRenderTarget = m_ppd2dRenderTargets[m_nSwapChainBufferIndex]->GetSize();

	switch (g_State)
	{
		case LOBBY:
		{
			m_pLobbyScene->UIRender();
			ChattingSystem::GetInstance()->ShowLobbyChatting();
			break;
		}

		case INGAME:
		{
			m_mapClients[m_currentBomberID].name;
			wchar_t name[16];
			int nLen = MultiByteToWideChar(CP_ACP, 0, m_mapClients[m_currentBomberID].name, strlen(m_mapClients[m_currentBomberID].name), NULL, NULL);
			MultiByteToWideChar(CP_ACP, 0, m_mapClients[m_currentBomberID].name, strlen(m_mapClients[m_currentBomberID].name), name, nLen);
			name[nLen] = '\0';

			m_pScene->UIRender(name);

			ShowIceCooltime();
			SetNamecard();
			//채팅
			ChattingSystem::GetInstance()->ShowIngameChatting(m_GameTimer.GetTimeElapsed());
			break;
		}

#ifdef _WITH_SERVER_
		case LOGIN:
		{
			if(m_pLoginScene)
				m_pLoginScene->GetIDInstance()->ShowIDInput(m_pd2dDeviceContext);
			break;
		}	

		case CONNECT:
		{
			if (m_pIPScene)
				m_pIPScene->UIRender(m_hWnd);
			break;
		}
#endif
	default:
		break;

	}

	////////////////////////////////////////////////////////////////////////
	//WITH_DIRECT2D_IMAGE_EFFECT

	////////////////////////////////////////////////////////////////////////
	m_pd2dDeviceContext->EndDraw();

	//Releases D3D11 resources that were wrapped for D3D 11on12
	m_pd3d11On12Device->ReleaseWrappedResources(&m_ppd3d11WrappedBackBuffers[m_nSwapChainBufferIndex], 1);

	//,커맨드 버퍼에 대기중인 커맨드를 gpu로 전송
	m_pd3d11DeviceContext->Flush();
}
#endif

#ifdef _WITH_SERVER_

void CGameFramework::ShowCurrentBomber()
{

	

}
void CGameFramework::ResetAnimationForRoundStart()
{
	if (m_pPlayer) 
	{
		m_pPlayer->m_pAnimationController->SetTrackAnimationSet(0, CAnimationController::IDLE);
		m_pPlayer->m_pAnimationController->SetAnimationState(CAnimationController::IDLE);
		m_pPlayer->m_pAnimationController->SetTrackPosition(0, 0.0f);

		m_pPlayer->SetUseGoldHammerTimeZero();
		m_pPlayer->SetGoldHammerUse(false);

	}
	if (m_pScene)
	{
		auto iter = m_pScene->getShaderManager()->getShaderMap().find("OtherPlayer");

		if (iter != m_pScene->getShaderManager()->getShaderMap().end())
		{

			//vector<pair<char, char>>& vec = dynamic_cast<CSkinnedAnimationObjectShader*>((*iter).second)->m_vMaterial;
			

			for (auto enemyID : m_mapClients)
			{
				(*iter).second->m_ppObjects[enemyID.first]->m_pAnimationController->SetTrackAnimationSet(0, CAnimationController::IDLE);
				(*iter).second->m_ppObjects[enemyID.first]->m_pAnimationController->SetAnimationState(CAnimationController::IDLE);
				(*iter).second->m_ppObjects[enemyID.first]->m_pAnimationController->SetTrackPosition(0, 0.0f);
			}

		}
	}
}

void CGameFramework::ProcessPacket(char* packet)
{
	switch (packet[1])
	{
	case SC_ACCESS_COMPLETE:
	{
		SC_PACKET_ACCESS_COMPLETE* pAC = reinterpret_cast<SC_PACKET_ACCESS_COMPLETE*>(packet);
		//플레이어 아이디 Set
		m_pPlayer->SetPlayerID(pAC->myId);
		Network::GetInstance()->SetMyID(pAC->myId);
		m_HostID = pAC->hostId;

		//reinterpret_cast<CCharacterSelectUIShader*>(m_pLobbyScene->GetCharacterSelectShader())->ChangeHost();

		CCharacterSelectUIShader::SetMyID(pAC->myId);

		//cout << "0. 접속 완료!! ▶ ";
		//cout << "MyID : " << (int)m_pPlayer->GetPlayerID() << ", HostID : " << m_HostID << endl;
		break;
	}

	case SC_ACCESS_PLAYER:
	{
		SC_PACKET_ACCESS_PLAYER* pAP = reinterpret_cast<SC_PACKET_ACCESS_PLAYER*>(packet);
		//cout << "1. OtherPlayer 접속!! ▶";
		//cout << "OtherPlayerID : " << (int)pAP->id << endl;
		break;
	}

	case SC_CHOSEN_CHARACTER:
	{
		SC_PACKET_CHOSEN_CHARACTER *pCC = reinterpret_cast<SC_PACKET_CHOSEN_CHARACTER *>(packet);

		for (int i = 0; i < MAX_USER; ++i)
		{
			CLobbyScene::AddClientsCharacter(i, pCC->matID[i]);
		//	cout << "2. Character of OtherPlayer - id: " << i << " : " << (int)pCC->matID[i] << endl;

			auto iter = m_pScene->getShaderManager()->getShaderMap().find("OtherPlayer");
			if (iter != m_pScene->getShaderManager()->getShaderMap().end())
			{
				if (pCC->matID[i] != -1 && m_pPlayer->GetPlayerID() != i)
					dynamic_cast<CSkinnedAnimationObjectShader*>((*iter).second)->MappingUserToEvilbear(i, pCC->matID[i]);
			}
		}

		break;
	}

	case SC_CLIENT_LOBBY_IN:
	{
		SC_PACKET_LOBBY_IN* pLI = reinterpret_cast<SC_PACKET_LOBBY_IN*>(packet);

		m_mapClients[(int)pLI->id] = pLI->client_state;
		m_mapClients[(int)pLI->id].isReady = pLI->client_state.isReady;
		m_mapClients[(int)pLI->id].id = pLI->id;

		//맵의 emplace는 한번 생성하면 똑같은 키에 value를 넣는 작업을 하지 않는다.(중복을 허용하지 않기 때문에)
		string user = m_mapClients[(int)pLI->id].name;
		string s = "님이 입장하였습니다.";

		ChattingSystem::GetInstance()->PushChattingText(user, s.c_str());

		//cout << "3. 로비에 접속" << endl;
		//cout << "m_mapClients 사이즈" << m_mapClients.size() << endl;
		break;
	}

	case SC_CHOICE_CHARACTER:
	{
		SC_PACKET_CHOICE_CHARACTER* pChoiceCharacter = reinterpret_cast<SC_PACKET_CHOICE_CHARACTER*>(packet);

		char playerID = pChoiceCharacter->id;
		char matID = pChoiceCharacter->matID;

		CLobbyScene::AddClientsCharacter(playerID, matID);

		if (playerID == m_pPlayer->GetPlayerID())
		{
			m_pPlayer->SetMaterialID(matID);
		}
		// 다른 플레이어 아이디일 때,
		else if (playerID < MAX_USER)
		{
			auto iter = m_pScene->getShaderManager()->getShaderMap().find("OtherPlayer");
			if (iter != m_pScene->getShaderManager()->getShaderMap().end())
			{
				dynamic_cast<CSkinnedAnimationObjectShader*>((*iter).second)->MappingUserToEvilbear(playerID, matID);
			}
		}
		break;
	}

	case SC_CLIENT_LOBBY_OUT:
	{
		//printf("SC_CLIENT_LOBBY_OUT 호출");
		SC_PACKET_LOBBY_OUT *pLO = reinterpret_cast<SC_PACKET_LOBBY_OUT*>(packet);

		if (pLO->id < MAX_USER)
		{
			char id = m_mapClients[pLO->id].id;
			// 나갔으면 캐릭터 초기화
			CLobbyScene::AddClientsCharacter(pLO->id, -1);

			m_mapClients[(int)pLO->id].isReady = false;
			string user = m_mapClients[(int)pLO->id].name;
			string s = "님이 나갔습니다.";

			ChattingSystem::GetInstance()->PushChattingText(user, s.c_str());
			strcpy(m_mapClients[(int)pLO->id].name, " ");

			auto mapIter = find_if(m_mapClients.begin(), m_mapClients.end(), [&id](const pair<char, clientsInfo>& p)
			{
				return p.first == id;
			});
			if (mapIter != m_mapClients.end())
			{
				m_mapClients.erase(mapIter);
			}

			//cout << "m_mapClients 사이즈" << m_mapClients.size() << endl;
		}
		break;
	}

	case SC_CHANGE_HOST_ID:
	{
		SC_PACKET_CHANGE_HOST *pCH = reinterpret_cast<SC_PACKET_CHANGE_HOST*>(packet);

		if (pCH->hostID < MAX_USER)
		{
			m_HostID = pCH->hostID;

			reinterpret_cast<CCharacterSelectUIShader*>(m_pLobbyScene->GetCharacterSelectShader())->ChangeHost();

			// 모든 클라의 레디를 해제함
			for (auto& client : m_mapClients)
				client.second.isReady = false;
		}

		break;
	}

	case SC_READY_STATE:
	{
		SC_PACKET_READY_STATE *pReady = reinterpret_cast<SC_PACKET_READY_STATE*>(packet);

		m_mapClients[pReady->id].isReady = true;

		//cout << "SC_READY_STATE 호출" << endl;
		break;
	}

	case SC_UNREADY_STATE:
	{
		SC_PACKET_UNREADY_STATE *pNotReady = reinterpret_cast<SC_PACKET_UNREADY_STATE*>(packet);

		m_mapClients[pNotReady->id].isReady = false;
		break;
	}

	case SC_CHATTING:
	{
		SC_PACKET_CHATTING *pCh = reinterpret_cast<SC_PACKET_CHATTING*>(packet);

		const string& clientName = m_mapClients[pCh->id].name;

		ChattingSystem::GetInstance()->ResetShowTime(0.0f);
		//ChattingSystem::GetInstance()->SetActive(true);
		ChattingSystem::GetInstance()->PushChattingText(clientName, pCh->message);
		break;
	}

	case SC_PLEASE_READY:
	{
		Network::GetInstance()->SetNullRS();

		string str = "모든 플레이어가 Ready하지 않았습니다.레디해주세요.";
		ChattingSystem::GetInstance()->PushText(str);
		//printf("모든 플레이어가 Ready하지 않았습니다.\n");
		break;
	}

	case SC_ROUND_START:
	{
		SC_PACKET_ROUND_START *pRS = reinterpret_cast<SC_PACKET_ROUND_START *>(packet);

		m_pScene->SetBomberID(pRS->bomberID);

		//애니메이션 리셋
		ResetAnimationForRoundStart();


		//라운드가 시작할 때 마다 플레이어의 아이템 소지를 모두 초기화 시켜야한다.
		m_pPlayer->SetIsBomb(false);
		m_pPlayer->setIsGoldHammer(false);
		m_pPlayer->setIsGoldTimer(false);
		m_pPlayer->SetIsHammer(false);
		m_pPlayer->SetIsICE(false);

		m_pPlayer->SetIsShowCoolTime(false);
		m_pPlayer->Sub_Inventory(CItem::ItemType::GoldHammer);
		m_pPlayer->Sub_Inventory(CItem::ItemType::NormalHammer);
		m_pPlayer->Sub_Inventory(CItem::ItemType::GoldTimer);

		m_currentBomberID = pRS->bomberID;
		if (pRS->bomberID == m_pPlayer->GetPlayerID())
		{
			m_pPlayer->SetIsBomb(true);

		}

		if (pRS->bomberID < MAX_USER)
		{
			auto iter = m_pScene->getShaderManager()->getShaderMap().find("OtherPlayer");

			if (iter != m_pScene->getShaderManager()->getShaderMap().end())
			{
				char bomber_id = m_mapClients[pRS->bomberID].id;

				for (auto enemy : m_mapClients)
				{
					if (enemy.second.id == bomber_id)
					{
						(*iter).second->m_ppObjects[bomber_id]->SetIsICE(false);
						(*iter).second->m_ppObjects[bomber_id]->SetIsBomb(true);
						(*iter).second->m_ppObjects[bomber_id]->setIsGoldTimer(false);
						(*iter).second->m_ppObjects[bomber_id]->setIsGoldHammer(false);
						(*iter).second->m_ppObjects[bomber_id]->SetIsHammer(false);
					}
					else
					{
						(*iter).second->m_ppObjects[enemy.second.id]->SetIsICE(false);
						(*iter).second->m_ppObjects[enemy.second.id]->SetIsBomb(false);
						(*iter).second->m_ppObjects[enemy.second.id]->setIsGoldHammer(false);
						(*iter).second->m_ppObjects[enemy.second.id]->setIsGoldTimer(false);
						(*iter).second->m_ppObjects[enemy.second.id]->SetIsHammer(false);


					}
				}
			}
		}

		auto itemIter = m_pScene->getShaderManager()->getShaderMap().find("Item");
		if (itemIter != m_pScene->getShaderManager()->getShaderMap().end())
		{
			m_pScene->SetGoldHammerCnt(pRS->goldHammerCnt);
			m_pScene->SetNormalHammerCnt(pRS->hammerCnt);
			m_pScene->SetGoldTimerCnt(pRS->goldTimerCnt);

			g_Round = pRS->round;
			switch (g_Round)
			{
			case ROUND_1:
				CSoundSystem::StopSound(CSoundSystem::SOUND_TYPE::FIRE_SOUND);
				ChattingSystem::GetInstance()->ClearChattingBox();
				break;
			case ROUND_2:
				CSoundSystem::StopSound(CSoundSystem::SOUND_TYPE::FIRE_SOUND);
				break;
			case ROUND_3:
				CSoundSystem::PlayingSound(CSoundSystem::SOUND_TYPE::FIRE_SOUND, 0.5f);
				break;
			default:
		//		cout << "미정의 라운드\n";
				break;
			}


		}

		clientCount = pRS->clientCount;
		if (m_pLobbyScene)
		{
			CSoundSystem::StopSound(CSoundSystem::LOBBY_BGM);
			CSoundSystem::StopSound(CSoundSystem::CHARACTER_SELECT);
		}

		g_State = INGAME;
		m_RoundStartTime = std::chrono::steady_clock::now();
		m_RoundCountTime = std::chrono::steady_clock::now();
		//시간을 받아야함.
		auto timerIter = m_pScene->getShaderManager()->getShaderMap().find("TimerUI");

		if (timerIter != m_pScene->getShaderManager()->getShaderMap().end())
		{
			dynamic_cast<CTimerUIShader*>((*timerIter).second)->ServerTimeAndSet(pRS->startTime);

		}
		ChattingSystem::GetInstance()->SetActive(false);

		auto explosionIter = m_pScene->getShaderManager()->getShaderMap().find("ExplosionParticle");
		if (explosionIter != m_pScene->getShaderManager()->getShaderMap().end())
		{
			dynamic_cast<CExplosionParticleShader*>((*explosionIter).second)->ResetParticles();
		}

		//printf("Round Start! Bomber is %d\n", m_mapClients[pRS->bomberID].id);

		break;
	}

	case SC_PUT_PLAYER:
	{
		SC_PACKET_PUT_PLAYER *pPP = reinterpret_cast<SC_PACKET_PUT_PLAYER*>(packet);
		// 앞으로 Put Player는 라운드 시작 시 캐릭터들의 시작 위치를 받는 패킷으로 사용
		// posIdx로 받는 인덱스 값을 이용해 시작 위치와 방향 값을 배열에서 가져와 적용 필요
		// 클라이언트 id값은 Lobby In 할 때 받는 id를 저장하면 될 듯. - 혜린

		// 아래 주석 코드는 PUT_PLAYER 부분이 아닌 InGame이 시작됐다는 패킷이 들어오면
		// 해줘야함- 명진.


		//   MappingUserToEvilbear(pPP->id, clientCount/*현재 접속한 유저 수를 받아야함 */);
		//cout <<"플레이어 ID-"<<(int)pPP->id<<",재질 -" <<(int)pPP->matID << "\n";
		//m_pPlayer->SetMaterialID(pPP->matID);   //플레이어 재질정   보 SET

		//XMFLOAT3 pos = CMapObjectsShader::spawn[g_Round][m_pPlayer->GetPlayerID()].pos;
		// 이런 식으로 시작위치 인덱스 사용하면 됨 posIdx[playerId]에는 각 플레이어의 시작위치인덱스가 담겨있음
		// ex) id가 3인 플레이어의 시작위치 => spawn[g_Round][pPP->posIdx[3]].pos;
		XMFLOAT3 pos = CMapObjectsShader::spawn[g_Round][pPP->posIdx[m_pPlayer->GetPlayerID()]].pos;

		m_pPlayer->SetPosition(pos);
		m_pPlayer->SetLookVector(XMFLOAT3(0.0f, 0.0f, 1.0f));
		m_pPlayer->SetUpVector(XMFLOAT3(0.0f, 1.0f, 0.0f));
		m_pPlayer->SetRightVector(XMFLOAT3(1.0f, 0.0f, 0.0f));

		//모든 아이템 보유 초기화
		m_pPlayer->setIsGoldHammer(false);
		m_pPlayer->setIsGoldTimer(false);
		m_pPlayer->SetIsHammer(false);

		auto iter = m_pScene->getShaderManager()->getShaderMap().find("OtherPlayer");
		if (iter != m_pScene->getShaderManager()->getShaderMap().end())
		{
			for (auto enemy : m_mapClients)
			{
				if (enemy.second.id == m_pPlayer->GetPlayerID())   continue;

				XMFLOAT3 pos = CMapObjectsShader::spawn[g_Round][pPP->posIdx[enemy.second.id]].pos;
				(*iter).second->m_ppObjects[enemy.second.id]->SetPosition(pos);
				(*iter).second->m_ppObjects[enemy.second.id]->SetLookVector(XMFLOAT3(0.0f, 0.0f, 1.0f));
				(*iter).second->m_ppObjects[enemy.second.id]->SetRightVector(XMFLOAT3(1.0f, 0.0f, 0.0f));
				(*iter).second->m_ppObjects[enemy.second.id]->SetUpVector(XMFLOAT3(0.0f, 1.0f, 0.0f));
				(*iter).second->m_ppObjects[enemy.second.id]->SetScale(10, 10, 10);
				//모든 아이템 보유 초기화
				(*iter).second->m_ppObjects[enemy.second.id]->setIsGoldTimer(false);
				(*iter).second->m_ppObjects[enemy.second.id]->setIsGoldHammer(false);
				(*iter).second->m_ppObjects[enemy.second.id]->SetIsHammer(false);
			}
		}
		break;
	}

	case SC_MOVE_PLAYER:
	{
		SC_PACKET_MOVE_PLAYER *pMP = reinterpret_cast<SC_PACKET_MOVE_PLAYER*>(packet);

		if (pMP->id == m_pPlayer->GetPlayerID())
		{
			XMFLOAT3 pos = XMFLOAT3(pMP->xPos, pMP->yPos, pMP->zPos);
			XMFLOAT3 look = XMFLOAT3(pMP->xLook, pMP->yLook, pMP->zLook);
			XMFLOAT3 up = XMFLOAT3(pMP->xUp, pMP->yUp, pMP->zUp);
			XMFLOAT3 right = XMFLOAT3(pMP->xRight, pMP->yRight, pMP->zRight);

			static float elapsedTime = 0.0f;

			//cout << "서버 위치 : " << pos.x << ", " << pos.y << ", " << pos.z << endl;
			//cout << "클라 위치 : " << m_pPlayer->GetPosition().x << ", " << m_pPlayer->GetPosition().y << ", " << m_pPlayer->GetPosition().z << endl;

			if (elapsedTime > 5.0f
				|| m_pPlayer->GetCollision() == true)
			{
				m_pPlayer->SetPosition(pos);
				elapsedTime = 0.0f;
			}
			else
			{
				elapsedTime += m_GameTimer.GetTimeElapsed();
			}
			//	m_pPlayer->SetPosition(pos);
			m_pPlayer->SetLookVector(look);
			m_pPlayer->SetUpVector(up);
			m_pPlayer->SetRightVector(right);
			//m_pPlayer->Rotate(pMP->pitch, pMP->yaw, pMP->roll);
			m_pPlayer->SetVelocityFromServer(pMP->fVelocity);
			m_pPlayer->SetMoveRotate(pMP->isMoveRotate);
			//m_pPlayer->SetCollision(false);
		}

		else if (pMP->id < MAX_USER)
		{
			char id = pMP->id;

			XMFLOAT3 pos = XMFLOAT3(pMP->xPos, pMP->yPos, pMP->zPos);
			XMFLOAT3 look = XMFLOAT3(pMP->xLook, pMP->yLook, pMP->zLook);
			XMFLOAT3 up = XMFLOAT3(pMP->xUp, pMP->yUp, pMP->zUp);
			XMFLOAT3 right = XMFLOAT3(pMP->xRight, pMP->yRight, pMP->zRight);

			auto iter = m_pScene->getShaderManager()->getShaderMap().find("OtherPlayer");
			if (iter != m_pScene->getShaderManager()->getShaderMap().end())
			{
				//char id = pMP->id;

				XMFLOAT3 pos = XMFLOAT3(pMP->xPos, pMP->yPos, pMP->zPos);
				XMFLOAT3 look = XMFLOAT3(pMP->xLook, pMP->yLook, pMP->zLook);
				XMFLOAT3 up = XMFLOAT3(pMP->xUp, pMP->yUp, pMP->zUp);
				XMFLOAT3 right = XMFLOAT3(pMP->xRight, pMP->yRight, pMP->zRight);

				auto iter = m_pScene->getShaderManager()->getShaderMap().find("OtherPlayer");
				if (iter != m_pScene->getShaderManager()->getShaderMap().end())
				{
					//vector<pair<char,char>>& vec = dynamic_cast<CSkinnedAnimationObjectShader*>((*iter).second)->m_vMaterial;
					char enemyID = m_mapClients[pMP->id].id;

					(*iter).second->m_ppObjects[enemyID]->SetPosition(pos);
					(*iter).second->m_ppObjects[enemyID]->SetLookVector(look);
					(*iter).second->m_ppObjects[enemyID]->SetRightVector(right);
					(*iter).second->m_ppObjects[enemyID]->SetUpVector(up);
					(*iter).second->m_ppObjects[enemyID]->SetScale(10, 10, 10);
					(*iter).second->m_ppObjects[enemyID]->SetVelocityFromServer(pMP->fVelocity);

				}

			}
		}
		break;
	}

	case SC_ANIMATION_INFO:
	{
		SC_PACKET_PLAYER_ANIMATION *pPA = reinterpret_cast<SC_PACKET_PLAYER_ANIMATION*>(packet);
		//cout <<"애니메이션 주최:" <<(int)pPA->id << "\n";
		if (pPA->id == m_pPlayer->GetPlayerID())
		{

			if (pPA->animation == CAnimationController::VICTORY)
			{
				if (m_pPlayer->m_pAnimationController->GetAnimationState() != CAnimationController::DIE)
				{
					m_pPlayer->SetTrackAnimationSet(0, pPA->animation);
					m_pPlayer->m_pAnimationController->SetAnimationState((CAnimationController::ANIMATIONTYPE)pPA->animation);
				}
			}


			//cout <<"애니메이션 : "<<(int)pPA->animation << "\n";
			//m_pPlayer->SetMoveRotate(false);
		}
		else if (pPA->id < MAX_USER)
		{
			char id = m_mapClients[pPA->id].id;

			char animNum = pPA->animation;
			auto iter = m_pScene->getShaderManager()->getShaderMap().find("OtherPlayer");

			if (iter != m_pScene->getShaderManager()->getShaderMap().end())
			{

				if ((CAnimationController::ANIMATIONTYPE)animNum == CAnimationController::RAISEHAND
					|| (CAnimationController::ANIMATIONTYPE)animNum == CAnimationController::DIE
					|| (CAnimationController::ANIMATIONTYPE)animNum == CAnimationController::USEGOLDHAMMER
					|| (CAnimationController::ANIMATIONTYPE)animNum == CAnimationController::ATTACK)
				{
					(*iter).second->m_ppObjects[id]->m_pAnimationController->SetTrackPosition(0, 0.0f);
				}

				if ((CAnimationController::ANIMATIONTYPE)animNum == CAnimationController::USEGOLDHAMMER)
				{
					(*iter).second->m_ppObjects[id]->SetIsLightEffect(true);
				}
				else
				{
					(*iter).second->m_ppObjects[id]->SetIsLightEffect(false);
				}

				if ((CAnimationController::ANIMATIONTYPE)animNum == CAnimationController::VICTORY)
				{
					if ((*iter).second->m_ppObjects[id]->m_pAnimationController->GetAnimationState() != CAnimationController::DIE)
					{
						(*iter).second->m_ppObjects[id]->SetTrackAnimationSet(0, animNum);
						(*iter).second->m_ppObjects[id]->m_pAnimationController->SetAnimationState((CAnimationController::ANIMATIONTYPE)animNum);
					}
				}
				else
				{
					(*iter).second->m_ppObjects[id]->SetTrackAnimationSet(0, animNum);
					(*iter).second->m_ppObjects[id]->m_pAnimationController->SetAnimationState((CAnimationController::ANIMATIONTYPE)animNum);
				}
			}
		}

		break;
	}

	case SC_STOP_RUN_ANIM:
	{
		SC_PACKET_STOP_RUN_ANIM *pSTA = reinterpret_cast<SC_PACKET_STOP_RUN_ANIM*>(packet);
		if (pSTA->id == m_pPlayer->GetPlayerID())
		{

			m_pPlayer->SetVelocityFromServer(0.0f);
		}

		//printf("SetVelocityFromServer\n");
		break;
	}

	case SC_REMOVE_PLAYER:
	{
		SC_PACKET_REMOVE_PLAYER *pRP = reinterpret_cast<SC_PACKET_REMOVE_PLAYER*>(packet);
		m_HostID = pRP->hostId;
		m_pScene->RemovePlayer(pRP->id);

		if (pRP->id != m_pPlayer->GetPlayerID())
		{
			auto iter = m_pScene->getShaderManager()->getShaderMap().find("OtherPlayer");

			if (iter != m_pScene->getShaderManager()->getShaderMap().end())
			{
				char id = m_mapClients[pRP->id].id;
				//vector<pair<char, char>>& vec = dynamic_cast<CSkinnedAnimationObjectShader*>((*iter).second)->m_vMaterial;

				(*iter).second->m_ppObjects[id]->SetPosition(0.0f, 0.0f, 0.0f);

				string s = "님이 나갔습니다.";
				string user = m_mapClients[id].name;

				auto mapIter = find_if(m_mapClients.begin(), m_mapClients.end(), [&id](const pair<char, clientsInfo>& p)
				{
					return p.first == id;
				});
				if (mapIter != m_mapClients.end())
				{
					m_mapClients.erase(mapIter);
				}
				//채팅 창에서 보여지는 시간 reset
				ChattingSystem::GetInstance()->ResetShowTime(0.0f);
				ChattingSystem::GetInstance()->PushChattingText(user, s.c_str());
			}
		}


		//printf("Player Disconnected ID : %d\n", pRP->id);
		break;
	}

	case SC_ROUND_END:
	{
		SC_PACKET_ROUND_END *pRE = reinterpret_cast<SC_PACKET_ROUND_END*>(packet);
		/*if (pRE->isWinner)
			printf("Win!\n");
		else
			printf("Lose..\n");*/


		m_IceChangeOk = false;
		//m_RoundCountTime = {};
		m_RoundStartTime = std::chrono::steady_clock::now() + 10s;



		if (m_pPlayer && m_pPlayer->GetIsBomb() == false)
		{
			m_pPlayer->m_pAnimationController->SetTrackAnimationSet(0, CAnimationController::VICTORY);
			m_pPlayer->m_pAnimationController->SetAnimationState(CAnimationController::VICTORY);
			Network::GetInstance()->SendAnimationState(CAnimationController::VICTORY);
			m_pPlayer->m_pAnimationController->SetTrackPosition(0, 0.0f);
		}

		// 점수 기록
		for (int i = 0; i < MAX_USER; ++i)
		{
			if (pRE->score[i] == -1)		continue;
			m_pScene->AddInGameScore(i, m_mapClients[i].name, pRE->score[i]);
		}

		// 점수로 순위 정렬
		m_pScene->SortInGameRank();
		break;
	}

	case SC_COMPARE_TIME:
	{
		SC_PACKET_COMPARE_TIME *pCT = reinterpret_cast<SC_PACKET_COMPARE_TIME*>(packet);

		auto iter = m_pScene->getShaderManager()->getShaderMap().find("TimerUI");
		if (iter != m_pScene->getShaderManager()->getShaderMap().end())
			dynamic_cast<CTimerUIShader*>((*iter).second)->ServerTimeAndSet(pCT->serverTime);
		break;
	}

	case SC_FREEZE:
	{
		SC_PACKET_FREEZE *pFR = reinterpret_cast<SC_PACKET_FREEZE*>(packet);

		if (pFR->id == m_pPlayer->GetPlayerID())
		{
			if (m_pPlayer->GetIsICE() == false)
			{
				//cout << "SetIsIce-true\n";
				m_pPlayer->SetIsICE(true);
				m_pPlayer->m_pAnimationController->SetTrackAnimationSet(0, CAnimationController::IDLE);
				m_pPlayer->m_pAnimationController->SetAnimationState(CAnimationController::ICE);
			}
		}

		else if (pFR->id < MAX_USER)
		{
			auto iter = m_pScene->getShaderManager()->getShaderMap().find("OtherPlayer");

			if (iter != m_pScene->getShaderManager()->getShaderMap().end())
			{
				char id = m_mapClients[pFR->id].id;

				if ((*iter).second->m_ppObjects[id]->GetIsICE() == false)
				{
					(*iter).second->m_ppObjects[id]->SetIsICE(true);

					(*iter).second->m_ppObjects[id]->m_pAnimationController->SetTrackAnimationSet(0, CAnimationController::IDLE);
					(*iter).second->m_ppObjects[id]->m_pAnimationController->SetAnimationState(CAnimationController::ICE);
				}
			}
		}
		break;
	}

	case SC_RELEASE_FREEZE:
	{
		SC_PACKET_RELEASE_FREEZE *pRF = reinterpret_cast<SC_PACKET_RELEASE_FREEZE*>(packet);

		if (pRF->id == m_pPlayer->GetPlayerID())
		{

			if (m_pPlayer->GetIsICE() == true)
			{
				//cout << "setIsIce = false\n";
				m_pPlayer->SetIsICE(false);
				//cout << "SetIsIce-false\n";
				m_pPlayer->m_pAnimationController->SetTrackAnimationSet(0, CAnimationController::IDLE);
				m_pPlayer->m_pAnimationController->SetAnimationState(CAnimationController::IDLE);
				m_pPlayer->m_pAnimationController->SetTrackPosition(0, 0.0f);
			}
		}
		else if (pRF->id < MAX_USER)
		{
			auto iter = m_pScene->getShaderManager()->getShaderMap().find("OtherPlayer");

			if (iter != m_pScene->getShaderManager()->getShaderMap().end())
			{
				char id = m_mapClients[pRF->id].id;
				//	vector<pair<char, char>>& vec = dynamic_cast<CSkinnedAnimationObjectShader*>((*iter).second)->m_vMaterial;

				if ((*iter).second->m_ppObjects[id]->GetIsICE() == true)
				{
					(*iter).second->m_ppObjects[id]->SetIsICE(false);
					(*iter).second->m_ppObjects[id]->m_pAnimationController->SetTrackAnimationSet(0, CAnimationController::IDLE);
					(*iter).second->m_ppObjects[id]->m_pAnimationController->SetAnimationState(CAnimationController::IDLE);
					(*iter).second->m_ppObjects[id]->m_pAnimationController->SetTrackPosition(0, 0.0f);
				}
			}
		}
		break;
	}
	case SC_GET_ITEM:
	{
		SC_PACKET_GET_ITEM *pGI = reinterpret_cast<SC_PACKET_GET_ITEM *>(packet);


		if (pGI->id == m_pPlayer->GetPlayerID() && m_pScene != nullptr)
		{
			string sItem = pGI->itemIndex;
			//cout << sItem<<"\n";

			int itemType = CItem::ItemType::Empty;

			m_pScene->MappingItemStringToItemType(sItem, itemType);

			m_pPlayer->Add_Inventory(sItem, itemType);

			switch (itemType)
			{
			case CItem::ItemType::GoldHammer:
				m_pPlayer->setIsGoldHammer(true);
				break;
			case CItem::ItemType::NormalHammer:
				m_pPlayer->SetIsHammer(true);
				break;
			case CItem::ItemType::GoldTimer:
				m_pPlayer->setIsGoldTimer(true);
				break;
			case CItem::ItemType::Empty:
			//	cout << "비정상적인 아이템\n";
				break;
			}
		}
		else if (pGI->id < MAX_USER && m_pScene != nullptr)
		{
			char id = m_mapClients[pGI->id].id;
			string sItem = pGI->itemIndex;
			//cout << sItem<<"\n";
			int itemType = CItem::ItemType::Empty;
			m_pScene->MappingItemStringToItemType(sItem, itemType);

			auto iter = m_pScene->getShaderManager()->getShaderMap().find("OtherPlayer");
			auto itemIter = m_pScene->getShaderManager()->getShaderMap().find("Item");

			if (iter != m_pScene->getShaderManager()->getShaderMap().end()
				&& itemIter != m_pScene->getShaderManager()->getShaderMap().end())
			{
				switch (itemType)
				{
				case CItem::ItemType::GoldHammer:
				{
					((*iter).second)->m_ppObjects[id]->setIsGoldHammer(true);
					break;
				}
				case CItem::ItemType::NormalHammer:
				{
					((*iter).second)->m_ppObjects[id]->SetIsHammer(true);

					break;
				}
				case CItem::ItemType::GoldTimer:
				{
					((*iter).second)->m_ppObjects[id]->setIsGoldTimer(true);
					break;
				}
				case CItem::ItemType::Empty:
				//	cout << "비정상적인 아이템\n";
					break;
				}
				CItemShader* pItem = (CItemShader*)(*itemIter).second;
				//cout << "상대방이 먹은 아이템" << sItem << "\n";
				pItem->ItemDelete(sItem);
			}


			//m_pPlayer->Add_Inventory(sItem,itemType);

		}

		break;
	}
	case SC_USE_ITEM:
	{
		SC_PACKET_USE_ITEM *pUI = reinterpret_cast<SC_PACKET_USE_ITEM*>(packet);

		switch (pUI->usedItem)
		{
		case ITEM::GOLD_TIMER:
		{
			//cout << "GOLD_TIMER 사용\n";

			if (pUI->id == m_pPlayer->GetPlayerID()) {
				if (m_pPlayer->GetSpecialInventory().size() > 0)
				{
					for (auto iter : m_pPlayer->GetSpecialInventory())
					{
						if (iter.second->getItemType() == CItem::ItemType::GoldTimer)
						{
							m_pPlayer->Sub_Inventory(CItem::ItemType::GoldTimer);
							m_pPlayer->setIsGoldTimer(false);
						}
					}

				}
			}
			else if (pUI->id < MAX_USER)
			{
				char id = m_mapClients[pUI->id].id;

				auto iter = m_pScene->getShaderManager()->getShaderMap().find("OtherPlayer");

				if (iter != m_pScene->getShaderManager()->getShaderMap().end())
				{
					(*iter).second->m_ppObjects[id]->setIsGoldTimer(false);
				}
			}
			break;
		}
		case ITEM::GOLD_HAMMER:
		{
			//cout << pUI->id << "가 Gold Hammer 아이템 사용\n";
			if (pUI->id == m_pPlayer->GetPlayerID())
			{
				m_pPlayer->Sub_Inventory(CItem::ItemType::GoldHammer);
				m_pPlayer->setIsGoldHammer(false);

				auto iceParticle = m_pScene->getShaderManager()->getShaderMap().find("CubeParticle");
				auto iter = m_pScene->getShaderManager()->getShaderMap().find("OtherPlayer");
				if (iter != m_pScene->getShaderManager()->getShaderMap().end())
				{

					for (auto enemyID : m_mapClients)
					{
						if ((*iter).second->m_ppObjects[enemyID.second.id]->GetIsICE())
						{
							dynamic_cast<CCubeParticleShader*>((*iceParticle).second)->SetParticleBlowUp((*iter).second->m_ppObjects[enemyID.second.id]->GetPosition(), enemyID.second.id);
							(*iter).second->m_ppObjects[enemyID.second.id]->SetIsICE(false);
							(*iter).second->m_ppObjects[enemyID.second.id]->m_pAnimationController->SetAnimationState(CAnimationController::IDLE);

						}
					}
				}

			}
			else if (pUI->id < MAX_USER)
			{
				char id = m_mapClients[pUI->id].id;

				auto iceParticle = m_pScene->getShaderManager()->getShaderMap().find("CubeParticle");
				if (m_pPlayer->GetIsICE() && iceParticle != m_pScene->getShaderManager()->getShaderMap().end())
				{
					dynamic_cast<CCubeParticleShader*>((*iceParticle).second)->SetParticleBlowUp(m_pPlayer->GetPosition(), m_pPlayer->GetPlayerID());
					m_pPlayer->SetIsICE(false);
					m_pPlayer->m_pAnimationController->SetAnimationState(CAnimationController::IDLE);
				}

				auto iter = m_pScene->getShaderManager()->getShaderMap().find("OtherPlayer");

				if (iter != m_pScene->getShaderManager()->getShaderMap().end())
				{

					(*iter).second->m_ppObjects[id]->setIsGoldHammer(false);

					for (auto enemyID : m_mapClients)
					{
						if ((*iter).second->m_ppObjects[enemyID.second.id]->GetIsICE())
						{
							dynamic_cast<CCubeParticleShader*>((*iceParticle).second)->SetParticleBlowUp((*iter).second->m_ppObjects[enemyID.second.id]->GetPosition(), enemyID.second.id);
							(*iter).second->m_ppObjects[enemyID.second.id]->SetIsICE(false);
							(*iter).second->m_ppObjects[enemyID.second.id]->m_pAnimationController->SetAnimationState(CAnimationController::IDLE);
						}
					}
				}
			}


			break;
		}
		case ITEM::NORMALHAMMER:
		{
		//	cout << m_mapClients[pUI->id].id << "가 " << m_mapClients[pUI->target].id << "에게 Normal Hammer 아이템 사용\n";
			//플레이어가 상대방을 때렸을때
			if (pUI->id < MAX_USER && pUI->target < MAX_USER)
			{
				if (pUI->id == m_pPlayer->GetPlayerID() && pUI->target != m_pPlayer->GetPlayerID())
				{
					//인벤토리에서 삭제
					m_pPlayer->Sub_Inventory(CItem::NormalHammer);
					m_pPlayer->SetIsHammer(false);

					auto iter = m_pScene->getShaderManager()->getShaderMap().find("OtherPlayer");
					auto iceParticle = m_pScene->getShaderManager()->getShaderMap().find("CubeParticle");
					if (iter != m_pScene->getShaderManager()->getShaderMap().end()
						&& iceParticle != m_pScene->getShaderManager()->getShaderMap().end())
					{


						if ((*iter).second->m_ppObjects[pUI->target]->GetIsICE())
						{
							dynamic_cast<CCubeParticleShader*>((*iceParticle).second)->SetParticleBlowUp((*iter).second->m_ppObjects[pUI->target]->GetPosition(), pUI->target);
							(*iter).second->m_ppObjects[pUI->target]->SetIsICE(false);
							(*iter).second->m_ppObjects[pUI->target]->m_pAnimationController->SetAnimationState(CAnimationController::IDLE);

						}

						m_pPlayer->SetCameraVibe(true);
						CSoundSystem::PlayingSound(CSoundSystem::ICE_BREAK);
					}

				}
				else if (pUI->id != m_pPlayer->GetPlayerID() && pUI->target == m_pPlayer->GetPlayerID())
				{
					m_pPlayer->SetIsICE(false);

					auto iter = m_pScene->getShaderManager()->getShaderMap().find("OtherPlayer");
					auto iceParticle = m_pScene->getShaderManager()->getShaderMap().find("CubeParticle");
					if (iter != m_pScene->getShaderManager()->getShaderMap().end()
						&& iceParticle != m_pScene->getShaderManager()->getShaderMap().end())
					{
						dynamic_cast<CCubeParticleShader*>((*iceParticle).second)->SetParticleBlowUp(m_pPlayer->GetPosition(), m_pPlayer->GetPlayerID());
						m_pPlayer->m_pAnimationController->SetAnimationState(CAnimationController::IDLE);

						(*iter).second->m_ppObjects[pUI->id]->SetIsHammer(false);

						m_pPlayer->SetCameraVibe(true);


						CSoundSystem::PlayingSound(CSoundSystem::ICE_BREAK);
					}



				}
				else
				{
					auto iter = m_pScene->getShaderManager()->getShaderMap().find("OtherPlayer");
					auto iceParticle = m_pScene->getShaderManager()->getShaderMap().find("CubeParticle");
					if (iter != m_pScene->getShaderManager()->getShaderMap().end()
						&& iceParticle != m_pScene->getShaderManager()->getShaderMap().end())
					{

						if ((*iter).second->m_ppObjects[pUI->target]->GetIsICE())
						{
							dynamic_cast<CCubeParticleShader*>((*iceParticle).second)->SetParticleBlowUp((*iter).second->m_ppObjects[pUI->target]->GetPosition(), pUI->target);
							(*iter).second->m_ppObjects[pUI->target]->SetIsICE(false);
							(*iter).second->m_ppObjects[pUI->target]->m_pAnimationController->SetAnimationState(CAnimationController::IDLE);

							(*iter).second->m_ppObjects[pUI->id]->SetIsHammer(false);
						}

						CSoundSystem::PlayingSound(CSoundSystem::ICE_BREAK);

					}
				}

			}


			break;
		}
		case ITEM::EMPTY:
		{
			break;
		}
		default:
		//	cout << "미정의 아이템\n";
			break;
		}

		break;
	}
	case SC_BOMB_EXPLOSION:
	{
		SC_PACKET_BOMB_EXPLOSION *pBE = reinterpret_cast<SC_PACKET_BOMB_EXPLOSION*>(packet);

		if (pBE->bomberId != m_pPlayer->GetPlayerID())
		{
			char id = m_mapClients[pBE->bomberId].id;
			auto bombIter = m_pScene->getShaderManager()->getShaderMap().find("Bomb");
			auto enemyIter = m_pScene->getShaderManager()->getShaderMap().find("OtherPlayer");
			auto explosionIter = m_pScene->getShaderManager()->getShaderMap().find("ExplosionParticle");
			if (bombIter != m_pScene->getShaderManager()->getShaderMap().end()
				&& enemyIter != m_pScene->getShaderManager()->getShaderMap().end()
				&& explosionIter != m_pScene->getShaderManager()->getShaderMap().end())
			{
				CBomb *pBomb = ((CBombParticleShader*)(*bombIter).second)->getBomb();

				pBomb->SetPosition((*enemyIter).second->m_ppObjects[id]->GetPosition());
				pBomb->setIsBlowing(true);

				//	dynamic_cast<CExplosionParticleShader*>((*explosionIter).second)->SetBlowingUp(true);

			//	cout << "적 터짐" << endl;
				dynamic_cast<CExplosionParticleShader*>((*explosionIter).second)->SetParticleBlowUp(pBomb->GetPosition());
			}
		}
		break;
	}
	case SC_ROLE_CHANGE:
	{
		SC_PACKET_ROLE_CHANGE *pRC = reinterpret_cast<SC_PACKET_ROLE_CHANGE*>(packet);

		m_currentBomberID = pRC->bomberId;
		m_pScene->SetBomberID(m_currentBomberID);

		//상대방이 자신에게 폭탄을 주었을 경우
		if (pRC->bomberId == m_pPlayer->GetPlayerID())
		{
			char runnerID = m_mapClients[pRC->runnerId].id;

			auto iter = m_pScene->getShaderManager()->getShaderMap().find("OtherPlayer");
			if (iter != m_pScene->getShaderManager()->getShaderMap().end())
			{
				m_pPlayer->SetUseGoldHammerTimeZero();
				m_pPlayer->SetGoldHammerUse(false);
				m_pPlayer->SetIsBomb(true);
				(*iter).second->m_ppObjects[runnerID]->SetIsBomb(false);

				m_pPlayer->SetIsShowCoolTime(false);
				if (m_pPlayer->GetIsLightEffect())
				{
					m_pPlayer->SetIsLightEffect(false);
				}
			}
		}
		else if (pRC->runnerId == m_pPlayer->GetPlayerID())		//자신이 상대방에게  폭탄을 넘겨줬을 경우
		{
			char bomberID = m_mapClients[pRC->bomberId].id;
			auto iter = m_pScene->getShaderManager()->getShaderMap().find("OtherPlayer");
			if (iter != m_pScene->getShaderManager()->getShaderMap().end())
			{
				m_pPlayer->SetIsBomb(false);
				(*iter).second->m_ppObjects[bomberID]->SetIsBomb(true);


			}
		}
		else if (pRC->bomberId != m_pPlayer->GetPlayerID() && pRC->runnerId != m_pPlayer->GetPlayerID()) //상대방끼리 폭탄을 주고받을 경우
		{
			char bomberId = m_mapClients[pRC->bomberId].id;
			char runnerId = m_mapClients[pRC->runnerId].id;
			auto iter = m_pScene->getShaderManager()->getShaderMap().find("OtherPlayer");
			if (iter != m_pScene->getShaderManager()->getShaderMap().end())
			{
				(*iter).second->m_ppObjects[runnerId]->SetIsBomb(false);

				(*iter).second->m_ppObjects[bomberId]->SetIsBomb(true);

			}
		}
		//cout << "술래 바뀜" << endl;
		CSoundSystem::PlayingSound(CSoundSystem::SOUND_TYPE::CATCH);

		break;
	}
	case SC_GO_LOBBY:
	{
		SC_PACKET_GO_LOBBY *pGL = reinterpret_cast<SC_PACKET_GO_LOBBY *>(packet);

		for (auto& client : m_mapClients)
			client.second.isReady = false;
		m_pLobbyScene->LobbySceneClear();
		m_pScene->InGameSceneClear(m_pd3dDevice, m_pd3dCommandList);

		g_State = LOBBY;

		//cout << "Go Lobby" << endl;
		break;
	}
	case SC_FREEZE_COOLTIME:
	{
		SC_PACKET_FREEZE_COOLTIME *pFC = reinterpret_cast<SC_PACKET_FREEZE_COOLTIME *>(packet);

		if (m_pPlayer)
		{
			m_pPlayer->SetIsShowCoolTime(true);
			m_pPlayer->SetIceCoolTime(pFC->cooltime);

		}
		break;
	}
	}
}

void CGameFramework::ProcessLogin()
{
	D3D12_VIEWPORT	d3dViewport = { 0, 0, FRAME_BUFFER_WIDTH , FRAME_BUFFER_HEIGHT, 0.0f, 1.0f };
	D3D12_RECT		d3dScissorRect = { 0, 0, FRAME_BUFFER_WIDTH , FRAME_BUFFER_HEIGHT };
	m_pd3dCommandList->RSSetViewports(1, &d3dViewport);
	m_pd3dCommandList->RSSetScissorRects(1, &d3dScissorRect);
	if (m_pLoginScene)
	{
		m_pLoginScene->Render(m_pd3dCommandList);
	}
	//::SynchronizeResourceTransition(m_pd3dCommandList, m_ppd3dSwapChainBackBuffers[m_nSwapChainBufferIndex], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		
	HRESULT hResult = m_pd3dCommandList->Close();

	ID3D12CommandList* ppd3dCommandLists[] = { m_pd3dCommandList };
	m_pd3dCommandQueue->ExecuteCommandLists(1, ppd3dCommandLists);
	
	WaitForGpuComplete();

#ifdef _WITH_DIRECT2D_

	ProcessDirect2D();

#endif
}
#endif

void CGameFramework::ProcessInGame(D3D12_CPU_DESCRIPTOR_HANDLE& d3dDsvDepthStencilBufferCPUHandle)
{

	CheckIceChangeOk();
	//카툰 렌더링 해야할 쉐이더들은 PreRender에서 그린다.
	
	if (m_pScene)
	{
		CSoundSystem::PlayingSound(CSoundSystem::INGAME_BGM, 0.5f);
		
		m_pScene->PreRender(m_pd3dCommandList, m_GameTimer.GetTimeElapsed(), m_pCamera);
	}

#ifdef _MAPTOOL_MODE_
		if (m_pMapToolShader)
		{
			m_pMapToolShader->Render(m_pd3dCommandList, m_pCamera, GameObject);
		}

#endif

#ifdef _WITH_PLAYER_TOP
		m_pd3dCommandList->ClearDepthStencilView(d3dDsvCPUDescriptorHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);
#endif

		if (m_pPlayer)
		{
			m_pPlayer->Animate(m_elapsedTime);
			m_pPlayer->UpdateTransform(NULL);
			m_pPlayer->Render(m_pd3dCommandList, m_pCamera, GameObject);
		}

		if (m_pScene)
			m_pScene->CheckObjectByObjectCollisions(m_elapsedTime);

		if (m_pCartoonShader != nullptr && g_OnCartoonShading == true)
		{
			m_pCartoonShader->SobelFilter(m_pd3dCommandList, m_ppd3dSwapChainBackBuffers[m_nSwapChainBufferIndex], m_ppd3dCartoonScreenRenderTargetBuffers, m_pCamera);
			m_pCartoonShader->Render(m_pd3dCommandList, m_ppd3dSwapChainBackBuffers[m_nSwapChainBufferIndex], m_ppd3dCartoonScreenRenderTargetBuffers, m_pCamera);
		}

		//카툰 렌더링 하지 않고 그려야할 쉐이더는 PostRender에서 그린다.
		if (m_pScene && m_pScene->getShaderManager())
		{
			m_pd3dCommandList->ClearDepthStencilView(d3dDsvDepthStencilBufferCPUHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);

			m_pd3dCommandList->OMSetRenderTargets(1, &m_pd3dRtvSwapChainBackBufferCPUHandles[m_nSwapChainBufferIndex], TRUE, &d3dDsvDepthStencilBufferCPUHandle);
			m_pScene->PostRender(m_pd3dCommandList, m_GameTimer.GetTimeElapsed(), m_pCamera);
		}

		//Direct2D를 사용하면 스왑체인 버퍼 리소스 전이를 Present로 바꿔주면 안된다. 
#ifndef _WITH_DIRECT2D_
	//if (m_bStart == false)
		::SynchronizeResourceTransition(m_pd3dCommandList, m_ppd3dSwapChainBackBuffers[m_nSwapChainBufferIndex], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
#endif
		HRESULT hResult = m_pd3dCommandList->Close();

		ID3D12CommandList* ppd3dCommandLists[] = { m_pd3dCommandList };
		m_pd3dCommandQueue->ExecuteCommandLists(1, ppd3dCommandLists);

		WaitForGpuComplete();

#ifdef _WITH_DIRECT2D_
		ProcessDirect2D();
#endif
}

void CGameFramework::ProcessLobby()
{
	D3D12_VIEWPORT	d3dViewport = { 0, 0, FRAME_BUFFER_WIDTH , FRAME_BUFFER_HEIGHT, 0.0f, 1.0f };
	D3D12_RECT		d3dScissorRect = { 0, 0, FRAME_BUFFER_WIDTH , FRAME_BUFFER_HEIGHT };
	m_pd3dCommandList->RSSetViewports(1, &d3dViewport);
	m_pd3dCommandList->RSSetScissorRects(1, &d3dScissorRect);

	static bool bStart = true;
	if (m_pLobbyScene)
	{
		CSoundSystem::PlayingSound(CSoundSystem::LOBBY_BGM, 0.5f);
		m_pLobbyScene->Render(m_pd3dCommandList);
	}

	//::SynchronizeResourceTransition(m_pd3dCommandList, m_ppd3dSwapChainBackBuffers[m_nSwapChainBufferIndex], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		
	HRESULT hResult = m_pd3dCommandList->Close();

	ID3D12CommandList* ppd3dCommandLists[] = { m_pd3dCommandList };
	m_pd3dCommandQueue->ExecuteCommandLists(1, ppd3dCommandLists);

	WaitForGpuComplete();

#ifdef _WITH_DIRECT2D_

	ProcessDirect2D();

#endif
}

void CGameFramework::FrameAdvance()
{
	m_GameTimer.Tick(60.0f);

	
	if (m_pPlayer)
		ProcessInput();

	AnimateObjects();

	float pfClearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

	HRESULT hResult = m_pd3dCommandAllocator->Reset();
	hResult = m_pd3dCommandList->Reset(m_pd3dCommandAllocator, NULL);

	::SynchronizeResourceTransition(m_pd3dCommandList, m_ppd3dSwapChainBackBuffers[m_nSwapChainBufferIndex], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

	m_pd3dRtvSwapChainBackBufferCPUHandles[m_nSwapChainBufferIndex] = m_pd3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_pd3dRtvSwapChainBackBufferCPUHandles[m_nSwapChainBufferIndex].ptr += (m_nSwapChainBufferIndex * m_nRtvDescriptorIncrementSize);

	m_pd3dCommandList->ClearRenderTargetView(m_pd3dRtvSwapChainBackBufferCPUHandles[m_nSwapChainBufferIndex], pfClearColor, 0, NULL);

	D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvDepthStencilBufferCPUHandle = m_pd3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_pd3dCommandList->ClearDepthStencilView(d3dDsvDepthStencilBufferCPUHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);

	m_pd3dCommandList->OMSetRenderTargets(1, &m_pd3dRtvSwapChainBackBufferCPUHandles[m_nSwapChainBufferIndex], TRUE, &d3dDsvDepthStencilBufferCPUHandle);

	switch(g_State)
	{
	case INGAME:
		{
			ProcessInGame(d3dDsvDepthStencilBufferCPUHandle);
			break;
		}

	case LOBBY:
		{
			ProcessLobby();
			break;
		}
#ifdef _WITH_SERVER_
	case LOGIN:
		{
			ProcessLogin();		
			break;
		}

	case CONNECT:
		{
			D3D12_VIEWPORT	d3dViewport = { 0, 0, FRAME_BUFFER_WIDTH , FRAME_BUFFER_HEIGHT, 0.0f, 1.0f };
			D3D12_RECT		d3dScissorRect = { 0, 0, FRAME_BUFFER_WIDTH , FRAME_BUFFER_HEIGHT };
			m_pd3dCommandList->RSSetViewports(1, &d3dViewport);
			m_pd3dCommandList->RSSetScissorRects(1, &d3dScissorRect);
	
			if(m_pIPScene)
				m_pIPScene->Render(m_pd3dCommandList);

			//::SynchronizeResourceTransition(m_pd3dCommandList, m_ppd3dSwapChainBackBuffers[m_nSwapChainBufferIndex], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		
			HRESULT hResult = m_pd3dCommandList->Close();

			ID3D12CommandList* ppd3dCommandLists[] = { m_pd3dCommandList };
			m_pd3dCommandQueue->ExecuteCommandLists(1, ppd3dCommandLists);

			WaitForGpuComplete();

	#ifdef _WITH_DIRECT2D_

			ProcessDirect2D();

	#endif
			break;
		}
#endif
	}

#ifdef _WITH_PRESENT_PARAMETERS
	DXGI_PRESENT_PARAMETERS dxgiPresentParameters;
	dxgiPresentParameters.DirtyRectsCount = 0;
	dxgiPresentParameters.pDirtyRects = NULL;
	dxgiPresentParameters.pScrollRect = NULL;
	dxgiPresentParameters.pScrollOffset = NULL;
	m_pdxgiSwapChain->Present1(1, 0, &dxgiPresentParameters);
#else

#ifdef _WITH_SYNCH_SWAPCHAIN
	m_pdxgiSwapChain->Present(1, 0);
#else
	m_pdxgiSwapChain->Present(0, 0);
#endif
#endif

	//	m_nSwapChainBufferIndex = m_pdxgiSwapChain->GetCurrentBackBufferIndex();
	MoveToNextFrame();

	m_GameTimer.GetFrameRate(m_pszFrameRate + 12, 37);
	size_t nLength = _tcslen(m_pszFrameRate);
	if (m_pPlayer)
	{
		XMFLOAT3 xmf3Position = m_pPlayer->GetPosition();
		_stprintf_s(m_pszFrameRate + nLength, 70 - nLength, _T(" (%4f, %4f, %4f)"), xmf3Position.x, xmf3Position.y, xmf3Position.z);
	}
	::SetWindowText(m_hWnd, m_pszFrameRate);
}

void CGameFramework::Worker_Thread()
{
	if (m_pLoadingCommandList)
		m_pLoadingCommandList->Reset(m_pLoadingCommandAllocator, nullptr);

	m_pLoadingScene = new CLoadingScene;
	m_pLoadingScene->BuildObjects(m_pd3dDevice, m_pLoadingCommandList);

	m_pLoadingCommandList->Close();
	ID3D12CommandList* ppd3dCommandLists[] = { m_pLoadingCommandList };
	m_pd3dCommandQueue->ExecuteCommandLists(1, ppd3dCommandLists);
	WaitForGpuComplete();

	m_GameTimer.Reset();

	if (m_pLoadingScene)
		m_pLoadingScene->ReleaseUploadBuffers();

	while (true)
	{
		m_GameTimer.Tick(0.0f);
		float elapsedTime = m_GameTimer.GetTimeElapsed();

		m_pLoadingScene->AnimateObjects(m_pLoadingCommandList, elapsedTime);

		float pfClearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

		HRESULT hResult = m_pLoadingCommandAllocator->Reset();
		hResult = m_pLoadingCommandList->Reset(m_pLoadingCommandAllocator, NULL);

		D3D12_VIEWPORT	d3dViewport = { 0, 0, FRAME_BUFFER_WIDTH , FRAME_BUFFER_HEIGHT, 0.0f, 1.0f };
		D3D12_RECT		d3dScissorRect = { 0, 0, FRAME_BUFFER_WIDTH , FRAME_BUFFER_HEIGHT };
		m_pLoadingCommandList->RSSetViewports(1, &d3dViewport);
		m_pLoadingCommandList->RSSetScissorRects(1, &d3dScissorRect);

		// 리소스 장벽(Barrier)
		D3D12_RESOURCE_BARRIER d3dResourceBarrier;
		::ZeroMemory(&d3dResourceBarrier, sizeof(D3D12_RESOURCE_BARRIER));
		d3dResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		d3dResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		d3dResourceBarrier.Transition.pResource = m_ppd3dSwapChainBackBuffers[m_nSwapChainBufferIndex];
		// StateBefore가 Present 상태가 되어야, DXGI가 Present를 실행한다.
		d3dResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;

		// StateAfter가 Render_Target 상태가 되면, Present가 끝나고 GPU가 그림을 바꿀 수 있다.
		d3dResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
		d3dResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		m_pLoadingCommandList->ResourceBarrier(1, &d3dResourceBarrier);

		D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = m_pd3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		d3dRtvCPUDescriptorHandle.ptr += (m_nSwapChainBufferIndex * m_nRtvDescriptorIncrementSize);
		D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle = m_pd3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

		m_pLoadingCommandList->OMSetRenderTargets(1, &d3dRtvCPUDescriptorHandle, FALSE, &d3dDsvCPUDescriptorHandle);

		m_pLoadingCommandList->ClearRenderTargetView(d3dRtvCPUDescriptorHandle, pfClearColor, 0, NULL);

		m_pLoadingCommandList->ClearDepthStencilView(d3dDsvCPUDescriptorHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);

		m_pLoadingScene->Render(m_pLoadingCommandList);

		d3dResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;


		d3dResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
		d3dResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		m_pLoadingCommandList->ResourceBarrier(1, &d3dResourceBarrier);

		m_pLoadingCommandList->Close();
		ID3D12CommandList* ppd3dCommandLists[] = { m_pLoadingCommandList };

		m_pd3dCommandQueue->ExecuteCommandLists(1, ppd3dCommandLists);
		WaitForGpuComplete();

		m_pdxgiSwapChain->Present(0, 0);

		MoveToNextFrame();

		// 파일 크기 전체 알 때까지는 스레드 기다리기
		while (g_IsloadingStart == false);

		double totalSize = g_TotalSize;
		double fileSize = g_FileSize;
		if (fileSize / totalSize >= 1)
			break;
	}
};

bool CGameFramework::IsInGame()
{ 
	bool ret;
	
	(g_State == INGAME) ? ret = true : ret = false;
				
	return ret;
}