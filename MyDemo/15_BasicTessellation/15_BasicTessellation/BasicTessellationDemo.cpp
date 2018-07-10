#include "Effects.h"
#include "Vertex.h"
#include "RenderStates.h"
#include "BasicTessellationDemo.h"

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	//上面是检测内存泄漏

	BasicTessellationDemo theApp(hInstance);

	if (!theApp.Init())
		return  0;
	return theApp.Run();
}

BasicTessellationDemo::BasicTessellationDemo(HINSTANCE hinstance):D3DApp(hinstance)
, mQuadPatchVB(0),
mEyePosW(0.0f, 0.0f, 0.0f), mTheta(1.3f*MathHelper::Pi), mPhi(0.2f*MathHelper::Pi), mRadius(80.0f)
{
	m_mainWndCaption = L"D3D11 曲面细分";
	m_isEnable4xMsaa = false;

	mLastMousePos.x = 0;
	mLastMousePos.y = 0;

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&mView, I);
	XMStoreFloat4x4(&mProj, I);

}


BasicTessellationDemo::~BasicTessellationDemo()
{
	m_pImmediateContext->ClearState();

	ReleaseCOM(mQuadPatchVB);

	Effects::DestroyAll();
	InputLayouts::DestroyAll();
	RenderStates::DestroyAll();
}

bool BasicTessellationDemo::Init()
{
	if (!D3DApp::Init())
	{
		return false;
	}
	//必须先初始化效果，因为输入布局的初始化依赖它
	Effects::InitAll(m_pD3dDevice);
	InputLayouts::InitAll(m_pD3dDevice);
	RenderStates::InitAll(m_pD3dDevice);

	BuildQuadPatchBuffer();

	return true;
}

void BasicTessellationDemo::OnResize()
{
	D3DApp::OnResize();

	XMMATRIX p = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, D3DApp::AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&mProj, p);

}

void BasicTessellationDemo::UpdateScene(float dt)
{
	// 视角变换矩阵
	// 将球面坐标转换为笛卡尔坐标
	float x = mRadius * sinf(mPhi)*cosf(mTheta);
	float z = mRadius * sinf(mPhi)*sinf(mTheta);
	float y = mRadius * cosf(mPhi);

	mEyePosW = XMFLOAT3(x, y, z);

	// 生成观察矩阵
	XMVECTOR pos = XMVectorSet(x, y, z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX V = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&mView, V);

}

void BasicTessellationDemo::DrawScene()
{
	m_pImmediateContext->ClearRenderTargetView(m_pRenderTargetView, reinterpret_cast<const float*>(&Colors::Silver));
	m_pImmediateContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	m_pImmediateContext->IASetInputLayout(InputLayouts::Basic32);
	m_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };

	XMMATRIX view = XMLoadFloat4x4(&mView);
	XMMATRIX proj = XMLoadFloat4x4(&mProj);
	XMMATRIX viewProj = view * proj;

	m_pImmediateContext->IASetInputLayout(InputLayouts::Pos);
	m_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);

	UINT stride = sizeof(Vertex::Pos);
	UINT offset = 0;

	// Set per frame constants.
	Effects::TessellationFX->SetEyePosW(mEyePosW);
	Effects::TessellationFX->SetFogColor(Colors::Silver);
	Effects::TessellationFX->SetFogStart(15.0f);
	Effects::TessellationFX->SetFogRange(175.0f);

	D3DX11_TECHNIQUE_DESC techDesc;
	Effects::TessellationFX->TessTech->GetDesc(&techDesc);

	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		m_pImmediateContext->IASetVertexBuffers(0, 1, &mQuadPatchVB, &stride, &offset);

		// Set per object constants.
		XMMATRIX world = XMMatrixIdentity();
		XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
		XMMATRIX worldViewProj = world * view*proj;

		Effects::TessellationFX->SetWorld(world);
		Effects::TessellationFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::TessellationFX->SetWorldViewProj(worldViewProj);
		Effects::TessellationFX->SetTexTransform(XMMatrixIdentity());
		//Effects::TessellationFX->SetMaterial(0);
		Effects::TessellationFX->SetDiffuseMap(0);

		Effects::TessellationFX->TessTech->GetPassByIndex(p)->Apply(0, m_pImmediateContext);

		m_pImmediateContext->RSSetState(RenderStates::WireframeRS);
		m_pImmediateContext->Draw(4, 0);
	}



	HR(m_pSwapChain->Present(0, 0));
}

void BasicTessellationDemo::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	// 一旦窗口捕获了鼠标，所有鼠标输入都针对该窗口，无论光标是否在窗口的边界内
	SetCapture(m_hMainWnd);
}

void BasicTessellationDemo::OnMouseUp(WPARAM btnState, int x, int y)
{
	// 不需要继续获得鼠标消息就释放掉
	ReleaseCapture();
}

void BasicTessellationDemo::OnMouseMove(WPARAM btnState, int x, int y)
{
	
	// 鼠标左键调整视角角度
	if ((btnState & MK_LBUTTON) != 0)
	{
		// 根据鼠标Pos和lastPos在x/y方向上的变化量dx/dy得出角度，每个像素的距离相当于1度，再将角度转为弧度
		float dx = XMConvertToRadians(0.25f*static_cast<float>(x - mLastMousePos.x));
		float dy = XMConvertToRadians(0.25f*static_cast<float>(y - mLastMousePos.y));

		// 根据dx和dy更新角度
		mTheta -= dx;
		mPhi -= dy;

		// 限制角度m_phi
		mPhi = MathHelper::Clamp(mPhi, 0.1f, MathHelper::Pi - 0.1f);
	}
	// 鼠标左键调整视角距离
	else if ((btnState & MK_RBUTTON) != 0)
	{
		// 使每个像素对应于场景中的0.005个单元
		float dx = 0.2f*static_cast<float>(x - mLastMousePos.x);
		float dy = 0.2f*static_cast<float>(y - mLastMousePos.y);

		// 根据鼠标输入更新摄像机半径
		mRadius -= dx - dy;

		// 限制m_radius
		mRadius = MathHelper::Clamp(mRadius, 5.0f, 300.0f);
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}

void BasicTessellationDemo::BuildQuadPatchBuffer()
{
	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(XMFLOAT3) * 4;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;

	XMFLOAT3 vertices[4] =
	{
		XMFLOAT3(-10.0f, 0.0f, +10.0f),
		XMFLOAT3(+10.0f, 0.0f, +10.0f),
		XMFLOAT3(-10.0f, 0.0f, -10.0f),
		XMFLOAT3(+10.0f, 0.0f, -10.0f)
	};

	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = vertices;
	HR(m_pD3dDevice->CreateBuffer(&vbd, &vinitData, &mQuadPatchVB));
}















