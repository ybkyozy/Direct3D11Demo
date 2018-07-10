#include "DrawBoxDemo.h"

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	//上面是检测内存泄漏

	DrawBoxDemo theApp(hInstance);

	if (!theApp.Init())
		return  0;
	return theApp.Run();
}

DrawBoxDemo::DrawBoxDemo(HINSTANCE hinstance):D3DApp(hinstance)
, m_pBoxVB(0)
, m_pBoxIB(0)
, m_pInputLayout(0)
, m_theta(1.5f*MathHelper::Pi)
, m_phi(0.25f*MathHelper::Pi)
, m_radius(5.0f)
, m_pFX(0)
, m_pTech(0)
, m_pFXWorldViewProj(0)
{
	m_mainWndCaption = L"D3D11 画多彩立方体";
	m_lastMousePos.x = 0;
	m_lastMousePos.y = 0;
	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&m_world, I);
	XMStoreFloat4x4(&m_view, I);
	XMStoreFloat4x4(&m_proj, I);
}


DrawBoxDemo::~DrawBoxDemo()
{
	ReleaseCOM(m_pBoxVB);
	ReleaseCOM(m_pBoxIB);
	ReleaseCOM(m_pInputLayout);
	ReleaseCOM(m_pBoxVB);
	ReleaseCOM(m_pBoxIB);
	ReleaseCOM(m_pFX);
}

bool DrawBoxDemo::Init()
{
	if (!D3DApp::Init())
	{
		return false;
	}
	BuildShader();
	BuildGeometryBuffers();
	return true;
}

void DrawBoxDemo::OnResize()
{
	D3DApp::OnResize();

	XMMATRIX proj = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, D3DApp::AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&m_proj, proj);

	//m_proj = Matrix::CreatePerspectiveFieldOfView(0.25f * MathHelper::Pi, D3DApp::AspectRatio(), 1.0f, 1000.0f);
}

void DrawBoxDemo::UpdateScene(float dt)
{
	// 视角变换矩阵
	// 将球面坐标转换为笛卡尔坐标
	float x = m_radius * sinf(m_phi)*cosf(m_theta);
	float z = m_radius * sinf(m_phi)*sinf(m_theta);
	float y = m_radius * cosf(m_phi);

	//Vector3 pos(x, y, z);
	//Vector3 target = Vector3::Zero;
	//Vector3 up(0, 1, 0);

	//m_view = Matrix::CreateLookAt(pos, target, up);

	//Matrix worldViewProj = m_world * m_proj * m_view;

	//m_pFXWorldViewProj->SetMatrix((float*)&worldViewProj);

	XMVECTOR pos = XMVectorSet(x, y, z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX V = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&m_view, V);

	// 把三个变换相乘，合成一个
	XMMATRIX world = XMLoadFloat4x4(&m_world);
	XMMATRIX view = XMLoadFloat4x4(&m_view);
	XMMATRIX proj = XMLoadFloat4x4(&m_proj);
	XMMATRIX worldViewProj = world * view*proj;

	// 通过C++程序更新Shader相应的变量
	m_pFXWorldViewProj->SetMatrix(reinterpret_cast<float*>(&worldViewProj));
}

void DrawBoxDemo::DrawScene()
{
	m_pImmediateContext->ClearRenderTargetView(m_pRenderTargetView, reinterpret_cast<const float*>(&Colors::LightSteelBlue));
	m_pImmediateContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// 指定输入布局、图元拓扑类型、顶点缓冲、索引缓冲、渲染状态
	m_pImmediateContext->IASetInputLayout(m_pInputLayout);
	m_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	m_pImmediateContext->IASetVertexBuffers(0, 1, &m_pBoxVB, &stride, &offset);
	m_pImmediateContext->IASetIndexBuffer(m_pBoxIB, DXGI_FORMAT_R32_UINT, 0);

	// 从technique获取pass并逐个渲染
	D3DX11_TECHNIQUE_DESC techDesc;
	m_pTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		m_pTech->GetPassByIndex(p)->Apply(0, m_pImmediateContext);
		m_pImmediateContext->DrawIndexed(36, 0, 0);// 立方体有36个索引
	}
	// 显示
	HR(m_pSwapChain->Present(0, 0));
}

void DrawBoxDemo::OnMouseDown(WPARAM btnState, int x, int y)
{
	m_lastMousePos.x = x;
	m_lastMousePos.y = y;

	// 一旦窗口捕获了鼠标，所有鼠标输入都针对该窗口，无论光标是否在窗口的边界内
	SetCapture(m_hMainWnd);
}

void DrawBoxDemo::OnMouseUp(WPARAM btnState, int x, int y)
{
	// 不需要继续获得鼠标消息就释放掉
	ReleaseCapture();
}

void DrawBoxDemo::OnMouseMove(WPARAM btnState, int x, int y)
{
	
	// 鼠标左键调整视角角度
	if ((btnState & MK_LBUTTON) != 0)
	{
		// 根据鼠标Pos和lastPos在x/y方向上的变化量dx/dy得出角度，每个像素的距离相当于1度，再将角度转为弧度
		float dx = XMConvertToRadians(0.25f*static_cast<float>(x - m_lastMousePos.x));
		float dy = XMConvertToRadians(0.25f*static_cast<float>(y - m_lastMousePos.y));

		// 根据dx和dy更新角度
		m_theta += dx;
		m_phi += dy;

		// 限制角度m_phi
		m_phi = MathHelper::Clamp(m_phi, 0.1f, MathHelper::Pi - 0.1f);
	}
	// 鼠标左键调整视角距离
	else if ((btnState & MK_RBUTTON) != 0)
	{
		// 使每个像素对应于场景中的0.005个单元
		float dx = 0.005f*static_cast<float>(x - m_lastMousePos.x);
		float dy = 0.005f*static_cast<float>(y - m_lastMousePos.y);

		// 根据鼠标输入更新摄像机半径
		m_radius += dx - dy;

		// 限制m_radius
		m_radius = MathHelper::Clamp(m_radius, 3.0f, 15.0f);
	}

	m_lastMousePos.x = x;
	m_lastMousePos.y = y;
}

void DrawBoxDemo::BuildGeometryBuffers()
{
	Vertex vertices[] =
	{
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) },
		{ XMFLOAT3(-1.0f, +1.0f, -1.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(+1.0f, +1.0f, -1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(+1.0f, -1.0f, -1.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, +1.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) },
		{ XMFLOAT3(-1.0f, +1.0f, +1.0f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(+1.0f, +1.0f, +1.0f), XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f) },
		{ XMFLOAT3(+1.0f, -1.0f, +1.0f), XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f) }
	//{ XMFLOAT3(-1.0f, -1.0f, -1.0f), (const float*)&Colors::White },
	//{ XMFLOAT3(-1.0f, +1.0f, -1.0f), (const float*)&Colors::Black },
	//{ XMFLOAT3(+1.0f, +1.0f, -1.0f), (const float*)&Colors::Red },
	//{ XMFLOAT3(+1.0f, -1.0f, -1.0f), (const float*)&Colors::Green },
	//{ XMFLOAT3(-1.0f, -1.0f, +1.0f), (const float*)&Colors::Blue },
	//{ XMFLOAT3(-1.0f, +1.0f, +1.0f), (const float*)&Colors::Yellow },
	//{ XMFLOAT3(+1.0f, +1.0f, +1.0f), (const float*)&Colors::Cyan },
	//{ XMFLOAT3(+1.0f, -1.0f, +1.0f), (const float*)&Colors::Magenta }
	};
	// 准备结构体，描述缓冲区
	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex) * 8;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;
	// 准备结构体，为缓冲区指定初始化数据
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = vertices;
	// 创建缓冲区
	HR(m_pD3dDevice->CreateBuffer(&vbd, &vinitData, &m_pBoxVB));

	/************************************************************************/
	/* 2.创建索引缓冲                                                       */
	/************************************************************************/
	UINT indices[] = {
		// 前表面
		0, 1, 2,
		0, 2, 3,

		// 后表面
		4, 6, 5,
		4, 7, 6,

		// 左表面
		4, 5, 1,
		4, 1, 0,

		// 右表面
		3, 2, 6,
		3, 6, 7,

		// 上表面
		1, 5, 6,
		1, 6, 2,

		// 下表面
		4, 0, 3,
		4, 3, 7
	};
	// 准备结构体，描述缓冲区
	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * 36;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;
	// 准备结构体，为缓冲区指定初始化数据
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = indices;
	// 创建缓冲区
	HR(m_pD3dDevice->CreateBuffer(&ibd, &iinitData, &m_pBoxIB));
}

void DrawBoxDemo::BuildShader()
{
	DWORD shaderFlags = 0;
#if defined( DEBUG ) || defined( _DEBUG )
	shaderFlags |= D3DCOMPILE_DEBUG;
	shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
	ID3D10Blob* compilationMsgs = 0;
	HR( D3DX11CompileEffectFromFile(L"Fx/Color.hlsl", nullptr, nullptr, shaderFlags, 0, m_pD3dDevice, &m_pFX, &compilationMsgs));
	// compilationMsgs中包含错误或警告信息
	if (compilationMsgs != 0)
	{
		OutputDebugStringA((char*)compilationMsgs->GetBufferPointer());
		ReleaseCOM(compilationMsgs);
	}

	// 从Effect中获取technique对象
	m_pTech = m_pFX->GetTechniqueByName("ColorTech");
	m_pFXWorldViewProj = m_pFX->GetVariableByName("gWorldViewProj")->AsMatrix();

	//创建输入布局 
	D3D11_INPUT_ELEMENT_DESC ied[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	UINT numLayoutElement = ARRAYSIZE(ied);
	// 从technique对象中获取pass信息
	D3DX11_PASS_DESC passDesc = { 0 };
	m_pTech->GetPassByIndex(0)->GetDesc(&passDesc);

	HR(m_pD3dDevice->CreateInputLayout(ied, numLayoutElement, passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize, &m_pInputLayout));
}

