#include <windowsx.h>//�����������õĺ�͸��ַ���Ĺ���
#include <sstream>
#include "D3DApp.h"

namespace
{
	D3DApp *g_d3dApp = NULL;
}

LRESULT CALLBACK MainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return g_d3dApp->MsgProc(hWnd, msg, wParam, lParam);
}


D3DApp::D3DApp(HINSTANCE hInstance) :
	m_hAppInst(hInstance),
	m_mainWndCaption(L"D3D11 Application"),
	m_d3dDriverType(D3D_DRIVER_TYPE_HARDWARE),
	m_clientHeight(768),
	m_clientWidth(1024),
	m_isEnable4xMsaa(false),
	m_hMainWnd(NULL),
	m_isAppPaused(false),
	m_isMaximized(false),
	m_isMinimized(false),
	m_isResizing(false),
	m_4xMsaaQuality(0),

	m_pD3dDevice(NULL),
	m_pImmediateContext(NULL),
	m_pSwapChain(NULL),
	m_pRenderTargetView(NULL),
	m_pDepthStencilBuffer(NULL)
{
	ZeroMemory(&m_screenViewPort, sizeof(D3D11_VIEWPORT));
	g_d3dApp = this;
}

D3DApp::~D3DApp()
{
	ReleaseCOM(m_pRenderTargetView);
	ReleaseCOM(m_pDepthStencilBuffer);
	ReleaseCOM(m_pSwapChain);
	ReleaseCOM(m_pDepthStencilView);

	if (m_pImmediateContext)
		m_pImmediateContext->ClearState();

	ReleaseCOM(m_pImmediateContext);
	ReleaseCOM(m_pD3dDevice);
}

HINSTANCE D3DApp::AppInst() const
{
	return m_hAppInst;
}

HWND D3DApp::MainWnd() const
{
	return m_hMainWnd;
}

float D3DApp::AspectRatio() const
{
	return static_cast<float>(m_clientWidth) / m_clientHeight;
}

int D3DApp::Run()
{
	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));
	m_timer.Reset();

	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			m_timer.Tick();
			if (!m_isAppPaused)
			{
				CalculateFrameStats();
				UpdateScene(m_timer.DeltaTime());
				DrawScene();
			}
			else
			{
				Sleep(100);
			}
		}
	}
	return static_cast<int>(msg.wParam);
}

bool D3DApp::Init()
{
	if (!InitMainWindow())
		return false;
	if (!InitDirect3D())
		return false;
	return true;
}

void D3DApp::OnResize()
{
	assert(m_pImmediateContext);
	assert(m_pD3dDevice);
	assert(m_pSwapChain);

	//release old views
	ReleaseCOM(m_pRenderTargetView);
	ReleaseCOM(m_pDepthStencilView);
	ReleaseCOM(m_pDepthStencilBuffer);

	HR(m_pSwapChain->ResizeBuffers(1, m_clientWidth, m_clientHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0));
	ID3D11Texture2D *backBuffer;
	HR(m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer)));
	HR(m_pD3dDevice->CreateRenderTargetView(backBuffer, 0, &m_pRenderTargetView));
	ReleaseCOM(backBuffer);

	//create depth/stencil buffer and view
	D3D11_TEXTURE2D_DESC depthStencilDecs;
	depthStencilDecs.Width = m_clientWidth;
	depthStencilDecs.Height = m_clientHeight;
	depthStencilDecs.MipLevels = 1;
	depthStencilDecs.ArraySize = 1;
	depthStencilDecs.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

	//�Ƿ�ʹ��4�ز���
	if (m_isEnable4xMsaa)
	{
		depthStencilDecs.SampleDesc.Count = 4;
		depthStencilDecs.SampleDesc.Quality = m_4xMsaaQuality - 1;
	}
	else
	{
		depthStencilDecs.SampleDesc.Count = 1;
		depthStencilDecs.SampleDesc.Quality = 0;
	}

	depthStencilDecs.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDecs.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDecs.CPUAccessFlags = 0;
	depthStencilDecs.MiscFlags = 0;

	HR(m_pD3dDevice->CreateTexture2D(&depthStencilDecs, 0, &m_pDepthStencilBuffer));
	HR(m_pD3dDevice->CreateDepthStencilView(m_pDepthStencilBuffer, 0, &m_pDepthStencilView));

	//���µ�render target view �� depth/stencil view������
	m_pImmediateContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);

	//����viewport
	m_screenViewPort.TopLeftX = 0;
	m_screenViewPort.TopLeftY = 0;
	m_screenViewPort.Width = static_cast<float>(m_clientWidth);
	m_screenViewPort.Height = static_cast<float>(m_clientHeight);
	m_screenViewPort.MaxDepth = 1.0f;
	m_screenViewPort.MinDepth = 0.0f;
	int so = sizeof(D3D11_SO_DECLARATION_ENTRY);
	m_pImmediateContext->RSSetViewports(1, &m_screenViewPort);
}

LRESULT D3DApp::MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		//��һ�����ڱ������ʧȥ����״̬
	case WM_ACTIVATE:
		if (LOWORD(wParam) == WA_INACTIVE)
		{
			m_isAppPaused = true;
			m_timer.Stop();
		}
		else
		{
			m_isAppPaused = false;
			m_timer.Start();
		}
		return 0;

		//���û��ػ洰��ʱ
	case WM_SIZE:
		m_clientWidth = LOWORD(lParam);
		m_clientHeight = HIWORD(lParam);
		if (m_pD3dDevice)
		{
			if (wParam == SIZE_MINIMIZED)//������С��
			{
				m_isAppPaused = true;
				m_isMinimized = true;
				m_isMaximized = false;
			}
			else if (wParam == SIZE_MAXIMIZED)//�������
			{
				m_isAppPaused = false;
				m_isMinimized = false;
				m_isMaximized = true;
				OnResize();
			}
			else if (wParam == SIZE_RESTORED)//���ڴ�С�ı�Ȳ������Ҳ������С��
			{
				if (m_isMinimized)
				{
					m_isAppPaused = false;
					m_isMinimized = false;
					OnResize();
				}
				else if (m_isMaximized)
				{
					m_isAppPaused = false;
					m_isMaximized = false;
					OnResize();
				}
				//���û����ڸı䴰�ڴ�Сʱ������OnResize(),���ı���ɺ���
				//����
				else if (m_isResizing)
				{

				}
				else
				{
					OnResize();
				}
			}
		}
		return 0;

		//�û���ʼ��ק�ı䴰�ڴ�С
	case WM_ENTERSIZEMOVE:
		m_isAppPaused = true;
		m_isResizing = true;
		m_timer.Stop();
		return 0;

		//�û��ı䴰�ڴ�С���
	case WM_EXITSIZEMOVE:
		m_isAppPaused = false;
		m_isResizing = false;
		m_timer.Start();
		OnResize();
		return  0;
		//��������
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

		//The WM_MENUCHAR message is sent when a menu is active and the user presses 
		// a key that does not correspond to any mnemonic or accelerator key.
	case WM_MENUCHAR:
		return MAKELRESULT(0, MNC_CLOSE);

		//��ֹ���ڱ��̫С
	case WM_GETMINMAXINFO:
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;
		return 0;

	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
		OnMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
		OnMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_MOUSEMOVE:
		OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

bool D3DApp::InitMainWindow()
{
	WNDCLASS wc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hbrBackground = static_cast<HBRUSH>(GetStockObject(NULL_BRUSH));
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hIcon = LoadIcon(0, IDI_APPLICATION);
	wc.hInstance = m_hAppInst;
	wc.lpfnWndProc = MainWndProc;
	wc.lpszClassName = L"D3DWndClassName";
	wc.lpszMenuName = NULL;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	if (!RegisterClass(&wc))
	{
		MessageBox(0, L"RegisterClass Failed", 0, 0);
		return false;
	}

	RECT rect{ 0, 0, m_clientWidth, m_clientHeight };
	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, false);
	int width = rect.right - rect.left;
	int height = rect.bottom - rect.top;

	m_hMainWnd = CreateWindow(L"D3DWndClassName", m_mainWndCaption.c_str(), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
		width, height, NULL, NULL, m_hAppInst, 0);
	if (!m_hMainWnd)
	{
		MessageBox(0, L"CreateWindow Failed", 0, 0);
		return 0;
	}

	ShowWindow(m_hMainWnd, SW_SHOW);
	UpdateWindow(m_hMainWnd);

	return true;
}

bool D3DApp::InitDirect3D()
{
	UINT createDeviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_FEATURE_LEVEL featureLevel;
	HRESULT hr = D3D11CreateDevice(
		0,//Ĭ�ϵ�adapter
		m_d3dDriverType,
		0,
		createDeviceFlags,
		0, 0,
		D3D11_SDK_VERSION,
		&m_pD3dDevice,
		&featureLevel,
		&m_pImmediateContext
	);
	if (FAILED(hr))
	{
		MessageBox(0, L"D3D11CreateDevice Failed", 0, 0);
		return hr;
	}

	if (featureLevel != D3D_FEATURE_LEVEL_11_0)
	{
		MessageBox(0, L"Direct3D Feature Level 11 unsupported!", 0, 0);
		return false;
	}

	//check 4x msaa quality support
	HR(m_pD3dDevice->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, 4, &m_4xMsaaQuality));
	assert(m_4xMsaaQuality > 0);

	//��佻��������
	DXGI_SWAP_CHAIN_DESC sd;
	sd.BufferDesc.Width = m_clientWidth;
	sd.BufferDesc.Height = m_clientHeight;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	//�Ƿ���4�ز���
	if (m_isEnable4xMsaa)
	{
		sd.SampleDesc.Count = 4;
		sd.SampleDesc.Quality = m_4xMsaaQuality - 1;
	}
	else
	{
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
	}

	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = 1;
	sd.OutputWindow = m_hMainWnd;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	sd.Flags = 0;

	//Ҫ��������������õ�IDXGIFactory
	IDXGIDevice *pDxgiDevice = 0;
	HR(m_pD3dDevice->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&pDxgiDevice)));

	IDXGIAdapter *pDxgiAdapter = 0;
	HR(pDxgiDevice->GetParent(__uuidof(IDXGIAdapter), reinterpret_cast<void**>(&pDxgiAdapter)));

	IDXGIFactory *pDxgiFactory = 0;
	HR(pDxgiAdapter->GetParent(__uuidof(IDXGIFactory), reinterpret_cast<void**>(&pDxgiFactory)));

	HR(pDxgiFactory->CreateSwapChain(m_pD3dDevice, &sd, &m_pSwapChain));

	ReleaseCOM(pDxgiDevice);
	ReleaseCOM(pDxgiAdapter);
	ReleaseCOM(pDxgiFactory);
	
	OnResize();

	return true;
}

void D3DApp::CalculateFrameStats()
{
	//����fps��ÿһ֡����
	static int frameCnt = 0;
	static float timeElapsed = 0.0f;

	frameCnt++;

	if ((m_timer.TotalTime() - timeElapsed) >= 1.0f)
	{
		float fps = static_cast<float>(frameCnt);
		float mspf = 1000.f / fps;

		std::wostringstream outs;
		outs.precision(6);//��������ʾ6λ
		outs << m_mainWndCaption << L"    " << L"FPS:" << fps << L"    "
			<< L"Frame Time:" << mspf << L" (ms) ";
		SetWindowText(m_hMainWnd, outs.str().c_str());

		frameCnt = 0;
		timeElapsed += 1.0f;
	}
}

void D3DApp::DrawScene()
{

}