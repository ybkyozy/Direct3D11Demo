#include "Effects.h"
#include "Vertex.h"
#include "RenderStates.h"
#include "MirrorDemo.h"

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	//上面是检测内存泄漏

	MirrorDemo theApp(hInstance);

	if (!theApp.Init())
		return  0;
	return theApp.Run();
}

MirrorDemo::MirrorDemo(HINSTANCE hinstance):D3DApp(hinstance)
, mRoomVB(0), mSkullVB(0), mSkullIB(0), mSkullIndexCount(0), mSkullTranslation(0.0f, 1.0f, -5.0f),
mFloorDiffuseMapSRV(0), mWallDiffuseMapSRV(0), mMirrorDiffuseMapSRV(0),
mEyePosW(0.0f, 0.0f, 0.0f), mRenderOptions(RenderOptions::Textures),
mTheta(1.24f*MathHelper::Pi), mPhi(0.42f*MathHelper::Pi), mRadius(12.0f)
{
	m_mainWndCaption = L"D3D11 模板之镜子";
	m_isEnable4xMsaa = false;

	mLastMousePos.x = 0;
	mLastMousePos.y = 0;

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&mRoomWorld, I);
	XMStoreFloat4x4(&mView, I);
	XMStoreFloat4x4(&mProj, I);

	mDirLights[0].Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mDirLights[0].Diffuse = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mDirLights[0].Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mDirLights[0].Direction = XMFLOAT3(0.57735f, -0.57735f, 0.57735f);

	mDirLights[1].Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mDirLights[1].Diffuse = XMFLOAT4(0.20f, 0.20f, 0.20f, 1.0f);
	mDirLights[1].Specular = XMFLOAT4(0.25f, 0.25f, 0.25f, 1.0f);
	mDirLights[1].Direction = XMFLOAT3(-0.57735f, -0.57735f, 0.57735f);

	mDirLights[2].Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mDirLights[2].Diffuse = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mDirLights[2].Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mDirLights[2].Direction = XMFLOAT3(0.0f, -0.707f, -0.707f);

	mRoomMat.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mRoomMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mRoomMat.Specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 16.0f);

	mSkullMat.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mSkullMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mSkullMat.Specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 16.0f);

	// Reflected material is transparent so it blends into mirror.
	mMirrorMat.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mMirrorMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f);
	mMirrorMat.Specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 16.0f);

	mShadowMat.Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mShadowMat.Diffuse = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.5f);
	mShadowMat.Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 16.0f);
}


MirrorDemo::~MirrorDemo()
{
	m_pImmediateContext->ClearState();
	ReleaseCOM(mRoomVB);
	ReleaseCOM(mSkullVB);
	ReleaseCOM(mSkullIB);
	ReleaseCOM(mFloorDiffuseMapSRV);
	ReleaseCOM(mWallDiffuseMapSRV);
	ReleaseCOM(mMirrorDiffuseMapSRV);

	Effects::DestroyAll();
	InputLayouts::DestroyAll();
	RenderStates::DestroyAll();
}

bool MirrorDemo::Init()
{
	if (!D3DApp::Init())
	{
		return false;
	}

	//必须先初始化效果，因为输入布局的初始化依赖它
	Effects::InitAll(m_pD3dDevice);
	InputLayouts::InitAll(m_pD3dDevice);
	RenderStates::InitAll(m_pD3dDevice);

	//载入纹理
	HR(CreateDDSTextureFromFile(m_pD3dDevice, L"Textures/checkboard.dds", nullptr, &mFloorDiffuseMapSRV));
	HR(CreateDDSTextureFromFile(m_pD3dDevice, L"Textures/brick01.dds", nullptr, &mWallDiffuseMapSRV));
	HR(CreateDDSTextureFromFile(m_pD3dDevice, L"Textures/ice.dds", nullptr, &mMirrorDiffuseMapSRV));

	BuildRoomGeometryBuffers();
	BuildSkullGeometryBuffers();
	return true;
}

void MirrorDemo::OnResize()
{
	D3DApp::OnResize();

	XMMATRIX p = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, D3DApp::AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&mProj, p);

}

void MirrorDemo::UpdateScene(float dt)
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

	// 根据按键选择渲染模式
	if (GetAsyncKeyState('1') & 0x8000)
		mRenderOptions = RenderOptions::Lighting;

	if (GetAsyncKeyState('2') & 0x8000)
		mRenderOptions = RenderOptions::Textures;

	if (GetAsyncKeyState('3') & 0x8000)
		mRenderOptions = RenderOptions::TexturesAndFog;


	// 允许用户移动骷髅
	if (GetAsyncKeyState('A') & 0x8000)
		mSkullTranslation.x -= 1.0f*dt;

	if (GetAsyncKeyState('D') & 0x8000)
		mSkullTranslation.x += 1.0f*dt;

	if (GetAsyncKeyState('W') & 0x8000)
		mSkullTranslation.y += 1.0f*dt;

	if (GetAsyncKeyState('S') & 0x8000)
		mSkullTranslation.y -= 1.0f*dt;

	// 不要让用户在地平面以下移动。
	mSkullTranslation.y = MathHelper::Max(mSkullTranslation.y, 0.0f);

	// 更新到新的矩阵
	XMMATRIX skullRotate = XMMatrixRotationY(0.5f*MathHelper::Pi);
	XMMATRIX skullScale = XMMatrixScaling(0.45f, 0.45f, 0.45f);
	XMMATRIX skullOffset = XMMatrixTranslation(mSkullTranslation.x, mSkullTranslation.y, mSkullTranslation.z);
	XMStoreFloat4x4(&mSkullWorld, skullRotate*skullScale*skullOffset);
}

void MirrorDemo::DrawScene()
{
	m_pImmediateContext->ClearRenderTargetView(m_pRenderTargetView, reinterpret_cast<const float*>(&Colors::Black));
	m_pImmediateContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	m_pImmediateContext->IASetInputLayout(InputLayouts::Basic32);
	m_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };

	UINT stride = sizeof(Vertex::Basic32);
	UINT offset = 0;

	XMMATRIX view = XMLoadFloat4x4(&mView);
	XMMATRIX proj = XMLoadFloat4x4(&mProj);
	XMMATRIX viewProj = view * proj;

	// 设置每帧常量缓冲区
	Effects::BasicFX->SetDirLights(mDirLights);
	Effects::BasicFX->SetEyePosW(mEyePosW);
	Effects::BasicFX->SetFogColor(Colors::Black);
	Effects::BasicFX->SetFogStart(2.0f);
	Effects::BasicFX->SetFogRange(40.0f);

	// 骷髅没有纹理坐标，所以我们不能纹理采样。
	ID3DX11EffectTechnique* activeTech = Effects::BasicFX->Light3Tech;
	ID3DX11EffectTechnique* activeSkullTech = Effects::BasicFX->Light3Tech;

	switch (mRenderOptions)
	{
	case RenderOptions::Lighting:
		activeTech = Effects::BasicFX->Light3Tech;
		activeSkullTech = Effects::BasicFX->Light3Tech;
		break;
	case RenderOptions::Textures:
		activeTech = Effects::BasicFX->Light3TexTech;
		activeSkullTech = Effects::BasicFX->Light3Tech;
		break;
	case RenderOptions::TexturesAndFog:
		activeTech = Effects::BasicFX->Light3TexFogTech;
		activeSkullTech = Effects::BasicFX->Light3FogTech;
		break;
	}

	D3DX11_TECHNIQUE_DESC techDesc;

	// 画正常的地板和墙壁。
	activeTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		ID3DX11EffectPass* pass = activeTech->GetPassByIndex(p);

		m_pImmediateContext->IASetVertexBuffers(0, 1, &mRoomVB, &stride, &offset);

		// 设置每个对象常量缓冲区
		XMMATRIX world = XMLoadFloat4x4(&mRoomWorld);
		XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
		XMMATRIX worldViewProj = world * view*proj;

		Effects::BasicFX->SetWorld(world);
		Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BasicFX->SetWorldViewProj(worldViewProj);
		Effects::BasicFX->SetTexTransform(XMMatrixIdentity());
		Effects::BasicFX->SetMaterial(mRoomMat);

		// 地板
		Effects::BasicFX->SetDiffuseMap(mFloorDiffuseMapSRV);
		pass->Apply(0, m_pImmediateContext);
		m_pImmediateContext->Draw(6, 0);

		// 墙壁
		Effects::BasicFX->SetDiffuseMap(mWallDiffuseMapSRV);
		pass->Apply(0, m_pImmediateContext);
		m_pImmediateContext->Draw(18, 6);
	}

	// 画正常的骷髅

	activeSkullTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		ID3DX11EffectPass* pass = activeSkullTech->GetPassByIndex(p);

		m_pImmediateContext->IASetVertexBuffers(0, 1, &mSkullVB, &stride, &offset);
		m_pImmediateContext->IASetIndexBuffer(mSkullIB, DXGI_FORMAT_R32_UINT, 0);

		XMMATRIX world = XMLoadFloat4x4(&mSkullWorld);
		XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
		XMMATRIX worldViewProj = world * view*proj;

		Effects::BasicFX->SetWorld(world);
		Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BasicFX->SetWorldViewProj(worldViewProj);
		Effects::BasicFX->SetMaterial(mSkullMat);

		pass->Apply(0, m_pImmediateContext);
		m_pImmediateContext->DrawIndexed(mSkullIndexCount, 0, 0);
	}

	// 画镜子到深度缓冲区

	activeTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		ID3DX11EffectPass* pass = activeTech->GetPassByIndex(p);

		m_pImmediateContext->IASetVertexBuffers(0, 1, &mRoomVB, &stride, &offset);

		// 设置每个对象常量缓冲区
		XMMATRIX world = XMLoadFloat4x4(&mRoomWorld);
		XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
		XMMATRIX worldViewProj = world * view*proj;

		Effects::BasicFX->SetWorld(world);
		Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BasicFX->SetWorldViewProj(worldViewProj);
		Effects::BasicFX->SetTexTransform(XMMatrixIdentity());

		// ！！！！！不写入渲染目标
		m_pImmediateContext->OMSetBlendState(RenderStates::NoRenderTargetWritesBS, blendFactor, 0xffffffff);

		//！！！！！将可见的镜子像素渲染为模板缓冲区。
		//！！！！！此时不要将镜子写入深度缓冲区，否则会遮挡反射。
		m_pImmediateContext->OMSetDepthStencilState(RenderStates::MarkMirrorDSS, 1);

		pass->Apply(0, m_pImmediateContext);
		m_pImmediateContext->Draw(6, 24);

		// 重置状态
		m_pImmediateContext->OMSetDepthStencilState(0, 0);
		m_pImmediateContext->OMSetBlendState(0, blendFactor, 0xffffffff);
	}


	//
	// 画反射的骷髅
	//
	activeSkullTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		ID3DX11EffectPass* pass = activeSkullTech->GetPassByIndex(p);

		m_pImmediateContext->IASetVertexBuffers(0, 1, &mSkullVB, &stride, &offset);
		m_pImmediateContext->IASetIndexBuffer(mSkullIB, DXGI_FORMAT_R32_UINT, 0);

		XMVECTOR mirrorPlane = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f); // xy plane
		XMMATRIX R = XMMatrixReflect(mirrorPlane);
		XMMATRIX world = XMLoadFloat4x4(&mSkullWorld) * R;
		XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
		XMMATRIX worldViewProj = world * view*proj;

		Effects::BasicFX->SetWorld(world);
		Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BasicFX->SetWorldViewProj(worldViewProj);
		Effects::BasicFX->SetMaterial(mSkullMat);

		// 保存原来的光线方向，并反射光线方向。
		XMFLOAT3 oldLightDirections[3];
		for (int i = 0; i < 3; ++i)
		{
			oldLightDirections[i] = mDirLights[i].Direction;

			XMVECTOR lightDir = XMLoadFloat3(&mDirLights[i].Direction);
			XMVECTOR reflectedLightDir = XMVector3TransformNormal(lightDir, R);
			XMStoreFloat3(&mDirLights[i].Direction, reflectedLightDir);
		}

		Effects::BasicFX->SetDirLights(mDirLights);

		// 倒影是顺时针三角形反射。
		m_pImmediateContext->RSSetState(RenderStates::CullClockwiseRS);

		// 仅将反射绘制到由模板缓冲区标记的可见镜像素中。
		m_pImmediateContext->OMSetDepthStencilState(RenderStates::DrawReflectionDSS, 1);
		pass->Apply(0, m_pImmediateContext);
		m_pImmediateContext->DrawIndexed(mSkullIndexCount, 0, 0);

		// 重置状态
		m_pImmediateContext->RSSetState(0);
		m_pImmediateContext->OMSetDepthStencilState(0, 0);

		// 还原光线方向
		for (int i = 0; i < 3; ++i)
		{
			mDirLights[i].Direction = oldLightDirections[i];
		}

		Effects::BasicFX->SetDirLights(mDirLights);
	}

	//
	// 像往常一样将镜子绘制到后台缓冲区，但具有透明度
	// 混合以便反射显示。
	// 

	activeTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		ID3DX11EffectPass* pass = activeTech->GetPassByIndex(p);

		m_pImmediateContext->IASetVertexBuffers(0, 1, &mRoomVB, &stride, &offset);

		XMMATRIX world = XMLoadFloat4x4(&mRoomWorld);
		XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
		XMMATRIX worldViewProj = world * view*proj;

		Effects::BasicFX->SetWorld(world);
		Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BasicFX->SetWorldViewProj(worldViewProj);
		Effects::BasicFX->SetTexTransform(XMMatrixIdentity());
		Effects::BasicFX->SetMaterial(mMirrorMat);
		Effects::BasicFX->SetDiffuseMap(mMirrorDiffuseMapSRV);

		// 画镜子
		m_pImmediateContext->OMSetBlendState(RenderStates::TransparentBS, blendFactor, 0xffffffff);
		pass->Apply(0, m_pImmediateContext);
		m_pImmediateContext->Draw(6, 24);
	}

	//
	// 画骷髅阴影
	//
	activeSkullTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		ID3DX11EffectPass* pass = activeSkullTech->GetPassByIndex(p);

		m_pImmediateContext->IASetVertexBuffers(0, 1, &mSkullVB, &stride, &offset);
		m_pImmediateContext->IASetIndexBuffer(mSkullIB, DXGI_FORMAT_R32_UINT, 0);

		XMVECTOR shadowPlane = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f); // xz plane
		XMVECTOR toMainLight = -XMLoadFloat3(&mDirLights[0].Direction);
		XMMATRIX S = XMMatrixShadow(shadowPlane, toMainLight);
		XMMATRIX shadowOffsetY = XMMatrixTranslation(0.0f, 0.001f, 0.0f); //比地面高一点点

		XMMATRIX world = XMLoadFloat4x4(&mSkullWorld)*S*shadowOffsetY;
		XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
		XMMATRIX worldViewProj = world * view*proj;

		Effects::BasicFX->SetWorld(world);
		Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BasicFX->SetWorldViewProj(worldViewProj);
		Effects::BasicFX->SetMaterial(mShadowMat);

		m_pImmediateContext->OMSetDepthStencilState(RenderStates::NoDoubleBlendDSS, 0);
		pass->Apply(0, m_pImmediateContext);
		m_pImmediateContext->DrawIndexed(mSkullIndexCount, 0, 0);

		m_pImmediateContext->OMSetBlendState(0, blendFactor, 0xffffffff);
		m_pImmediateContext->OMSetDepthStencilState(0, 0);
	}


	HR(m_pSwapChain->Present(0, 0));
}

void MirrorDemo::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	// 一旦窗口捕获了鼠标，所有鼠标输入都针对该窗口，无论光标是否在窗口的边界内
	SetCapture(m_hMainWnd);
}

void MirrorDemo::OnMouseUp(WPARAM btnState, int x, int y)
{
	// 不需要继续获得鼠标消息就释放掉
	ReleaseCapture();
}

void MirrorDemo::OnMouseMove(WPARAM btnState, int x, int y)
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
		float dx = 0.01f*static_cast<float>(x - mLastMousePos.x);
		float dy = 0.01f*static_cast<float>(y - mLastMousePos.y);

		// 根据鼠标输入更新摄像机半径
		mRadius -= dx - dy;

		// 限制m_radius
		mRadius = MathHelper::Clamp(mRadius, 3.0f, 50.0f);
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}

void MirrorDemo::BuildRoomGeometryBuffers()
{
	// Create and specify geometry.  For this sample we draw a floor
	// and a wall with a mirror on it.  We put the floor, wall, and
	// mirror geometry in one vertex buffer.
	//
	//   |--------------|
	//   |              |
	//   |----|----|----|
	//   |Wall|Mirr|Wall|
	//   |    | or |    |
	//   /--------------/
	//  /   Floor      /
	// /--------------/


	Vertex::Basic32 v[30];

	// Floor: Observe we tile texture coordinates.
	v[0] = Vertex::Basic32(-3.5f, 0.0f, -10.0f, 0.0f, 1.0f, 0.0f, 0.0f, 4.0f);
	v[1] = Vertex::Basic32(-3.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f);
	v[2] = Vertex::Basic32(7.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 4.0f, 0.0f);

	v[3] = Vertex::Basic32(-3.5f, 0.0f, -10.0f, 0.0f, 1.0f, 0.0f, 0.0f, 4.0f);
	v[4] = Vertex::Basic32(7.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 4.0f, 0.0f);
	v[5] = Vertex::Basic32(7.5f, 0.0f, -10.0f, 0.0f, 1.0f, 0.0f, 4.0f, 4.0f);

	// Wall: Observe we tile texture coordinates, and that we
	// leave a gap in the middle for the mirror.
	v[6] = Vertex::Basic32(-3.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 2.0f);
	v[7] = Vertex::Basic32(-3.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
	v[8] = Vertex::Basic32(-2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.5f, 0.0f);

	v[9] = Vertex::Basic32(-3.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 2.0f);
	v[10] = Vertex::Basic32(-2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.5f, 0.0f);
	v[11] = Vertex::Basic32(-2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.5f, 2.0f);

	v[12] = Vertex::Basic32(2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 2.0f);
	v[13] = Vertex::Basic32(2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
	v[14] = Vertex::Basic32(7.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 2.0f, 0.0f);

	v[15] = Vertex::Basic32(2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 2.0f);
	v[16] = Vertex::Basic32(7.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 2.0f, 0.0f);
	v[17] = Vertex::Basic32(7.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 2.0f, 2.0f);

	v[18] = Vertex::Basic32(-3.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
	v[19] = Vertex::Basic32(-3.5f, 6.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
	v[20] = Vertex::Basic32(7.5f, 6.0f, 0.0f, 0.0f, 0.0f, -1.0f, 6.0f, 0.0f);

	v[21] = Vertex::Basic32(-3.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
	v[22] = Vertex::Basic32(7.5f, 6.0f, 0.0f, 0.0f, 0.0f, -1.0f, 6.0f, 0.0f);
	v[23] = Vertex::Basic32(7.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 6.0f, 1.0f);

	// Mirror
	v[24] = Vertex::Basic32(-2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
	v[25] = Vertex::Basic32(-2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
	v[26] = Vertex::Basic32(2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);

	v[27] = Vertex::Basic32(-2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
	v[28] = Vertex::Basic32(2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);
	v[29] = Vertex::Basic32(2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f);

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex::Basic32) * 30;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = v;
	HR(m_pD3dDevice->CreateBuffer(&vbd, &vinitData, &mRoomVB));
}

void MirrorDemo::BuildSkullGeometryBuffers()
{
	std::ifstream fin("Models/skull.txt");

	if (!fin)
	{
		MessageBox(0, L"Models/skull.txt not found.", 0, 0);
		return;
	}

	UINT vcount = 0;
	UINT tcount = 0;
	std::string ignore;

	fin >> ignore >> vcount;
	fin >> ignore >> tcount;
	fin >> ignore >> ignore >> ignore >> ignore;

	std::vector<Vertex::Basic32> vertices(vcount);
	for (UINT i = 0; i < vcount; ++i)
	{
		fin >> vertices[i].Pos.x >> vertices[i].Pos.y >> vertices[i].Pos.z;
		fin >> vertices[i].Normal.x >> vertices[i].Normal.y >> vertices[i].Normal.z;
	}

	fin >> ignore;
	fin >> ignore;
	fin >> ignore;

	mSkullIndexCount = 3 * tcount;
	std::vector<UINT> indices(mSkullIndexCount);
	for (UINT i = 0; i < tcount; ++i)
	{
		fin >> indices[i * 3 + 0] >> indices[i * 3 + 1] >> indices[i * 3 + 2];
	}

	fin.close();

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex::Basic32) * vcount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	HR(m_pD3dDevice->CreateBuffer(&vbd, &vinitData, &mSkullVB));


	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * mSkullIndexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &indices[0];
	HR(m_pD3dDevice->CreateBuffer(&ibd, &iinitData, &mSkullIB));
}












