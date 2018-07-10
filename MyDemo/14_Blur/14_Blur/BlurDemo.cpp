#include "Effects.h"
#include "Vertex.h"
#include "RenderStates.h"
#include "BlurDemo.h"

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	//上面是检测内存泄漏

	BlurDemo theApp(hInstance);

	if (!theApp.Init())
		return  0;
	return theApp.Run();
}

BlurDemo::BlurDemo(HINSTANCE hinstance):D3DApp(hinstance)
, mLandVB(0), mLandIB(0), mWavesVB(0), mWavesIB(0),
mBoxVB(0), mBoxIB(0), mScreenQuadVB(0), mScreenQuadIB(0),
mGrassMapSRV(0), mWavesMapSRV(0), mCrateSRV(0), mOffscreenSRV(0), mOffscreenUAV(0), mOffscreenRTV(0),
mWaterTexOffset(0.0f, 0.0f), mEyePosW(0.0f, 0.0f, 0.0f), mLandIndexCount(0), mWaveIndexCount(0),
mRenderOptions(RenderOptions::TexturesAndFog),
mTheta(1.3f*MathHelper::Pi), mPhi(0.4f*MathHelper::Pi), mRadius(80.0f)
{
	m_mainWndCaption = L"D3D11 计算着色器之模糊";
	m_isEnable4xMsaa = false;

	mLastMousePos.x = 0;
	mLastMousePos.y = 0;

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&mLandWorld, I);
	XMStoreFloat4x4(&mWavesWorld, I);
	XMStoreFloat4x4(&mView, I);
	XMStoreFloat4x4(&mProj, I);

	XMMATRIX boxScale = XMMatrixScaling(15.0f, 15.0f, 15.0f);
	XMMATRIX boxOffset = XMMatrixTranslation(8.0f, 5.0f, -15.0f);
	XMStoreFloat4x4(&mBoxWorld, boxScale*boxOffset);

	XMMATRIX grassTexScale = XMMatrixScaling(5.0f, 5.0f, 0.0f);
	XMStoreFloat4x4(&mGrassTexTransform, grassTexScale);

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

	mLandMat.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mLandMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mLandMat.Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);

	mWavesMat.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mWavesMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f);
	mWavesMat.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 32.0f);

	mBoxMat.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mBoxMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mBoxMat.Specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 16.0f);
}


BlurDemo::~BlurDemo()
{
	m_pImmediateContext->ClearState();

	ReleaseCOM(mLandVB);
	ReleaseCOM(mLandIB);
	ReleaseCOM(mWavesVB);
	ReleaseCOM(mWavesIB);
	ReleaseCOM(mBoxVB);
	ReleaseCOM(mBoxIB);
	ReleaseCOM(mScreenQuadVB);
	ReleaseCOM(mScreenQuadIB);

	ReleaseCOM(mGrassMapSRV);
	ReleaseCOM(mWavesMapSRV);
	ReleaseCOM(mCrateSRV);

	ReleaseCOM(mOffscreenSRV);
	ReleaseCOM(mOffscreenUAV);
	ReleaseCOM(mOffscreenRTV);

	Effects::DestroyAll();
	InputLayouts::DestroyAll();
	RenderStates::DestroyAll();
}

bool BlurDemo::Init()
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
	HR(CreateDDSTextureFromFile(m_pD3dDevice, L"Textures/grass.dds", nullptr, &mGrassMapSRV));
	HR(CreateDDSTextureFromFile(m_pD3dDevice, L"Textures/water2.dds", nullptr, &mWavesMapSRV));
	HR(CreateDDSTextureFromFile(m_pD3dDevice, L"Textures/WireFence.dds", nullptr, &mCrateSRV));

	mWaves.Init(160, 160, 1.0f, 0.03f, 5.0f, 0.3f);

	BuildLandGeometryBuffers();
	BuildWaveGeometryBuffers();
	BuildCrateGeometryBuffers();
	BuildScreenQuadGeometryBuffers();
	BuildOffscreenViews();
	return true;
}

void BlurDemo::OnResize()
{
	D3DApp::OnResize();
	//重新创建依赖于客户区大小的资源。
	BuildOffscreenViews();
	mBlur.Init(m_pD3dDevice, m_clientWidth, m_clientHeight, DXGI_FORMAT_R8G8B8A8_UNORM);

	XMMATRIX p = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, D3DApp::AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&mProj, p);

}

void BlurDemo::UpdateScene(float dt)
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

	//
	// 每一秒钟生成一个随机波浪
	//
	static float t_base = 0.0f;
	if ((m_timer.TotalTime() - t_base) >= 0.1f)
	{
		t_base += 0.1f;

		DWORD i = 5 + rand() % (mWaves.RowCount() - 10);
		DWORD j = 5 + rand() % (mWaves.ColumnCount() - 10);

		float r = MathHelper::RandF(0.5f, 1.0f);

		mWaves.Disturb(i, j, r);
	}

	mWaves.Update(dt);

	//
	// 更新水的顶点缓冲区
	//

	D3D11_MAPPED_SUBRESOURCE mappedData;
	HR(m_pImmediateContext->Map(mWavesVB, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));

	Vertex::Basic32* v = reinterpret_cast<Vertex::Basic32*>(mappedData.pData);
	for (UINT i = 0; i < mWaves.VertexCount(); ++i)
	{
		v[i].Pos = mWaves[i];
		v[i].Normal = mWaves.Normal(i);

		// Derive tex-coords in [0,1] from position.
		v[i].Tex.x = 0.5f + mWaves[i].x / mWaves.Width();
		v[i].Tex.y = 0.5f - mWaves[i].z / mWaves.Depth();
	}

	m_pImmediateContext->Unmap(mWavesVB, 0);

	//
	// 动画水纹理坐标。
	//

	// 平铺水的纹理
	XMMATRIX wavesScale = XMMatrixScaling(5.0f, 5.0f, 0.0f);

	// 根据时间平移纹理
	mWaterTexOffset.y += 0.05f*dt;
	mWaterTexOffset.x += 0.1f*dt;
	XMMATRIX wavesOffset = XMMatrixTranslation(mWaterTexOffset.x, mWaterTexOffset.y, 0.0f);

	// 合并缩放和平移
	XMStoreFloat4x4(&mWaterTexTransform, wavesScale*wavesOffset);

	//
	// 键盘输入选择渲染模式
	//
	if (GetAsyncKeyState('1') & 0x8000)
		mRenderOptions = RenderOptions::Lighting;

	if (GetAsyncKeyState('2') & 0x8000)
		mRenderOptions = RenderOptions::Textures;

	if (GetAsyncKeyState('3') & 0x8000)
		mRenderOptions = RenderOptions::TexturesAndFog;

}

void BlurDemo::DrawScene()
{
	ID3D11RenderTargetView* renderTargets[1] = { mOffscreenRTV };
	m_pImmediateContext->OMSetRenderTargets(1, renderTargets, m_pDepthStencilView);

	m_pImmediateContext->ClearRenderTargetView(mOffscreenRTV, reinterpret_cast<const float*>(&Colors::Silver));
	m_pImmediateContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	//
	// 画场景到离屏纹理
	//

	DrawWrapper();

	//
	// 恢复后台缓冲区。 离屏渲染目标将作为输入
	// 计算着色器模糊，所以我们必须在我们之前解除它与OM阶段的绑定
	// 可以将其用作计算着色器的输入。
	//
	renderTargets[0] = m_pRenderTargetView;
	m_pImmediateContext->OMSetRenderTargets(1, renderTargets, m_pDepthStencilView);

	//mBlur.SetGaussianWeights(4.0f);
	mBlur.BlurInPlace(m_pImmediateContext, mOffscreenSRV, mOffscreenUAV, 4);

	//
	// 画全屏，这个资源视图已经经过模糊
	//

	m_pImmediateContext->ClearRenderTargetView(m_pRenderTargetView, reinterpret_cast<const float*>(&Colors::Silver));
	m_pImmediateContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	DrawScreenQuad();


	HR(m_pSwapChain->Present(0, 0));
}

void BlurDemo::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	// 一旦窗口捕获了鼠标，所有鼠标输入都针对该窗口，无论光标是否在窗口的边界内
	SetCapture(m_hMainWnd);
}

void BlurDemo::OnMouseUp(WPARAM btnState, int x, int y)
{
	// 不需要继续获得鼠标消息就释放掉
	ReleaseCapture();
}

void BlurDemo::OnMouseMove(WPARAM btnState, int x, int y)
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
		float dx = 0.1f*static_cast<float>(x - mLastMousePos.x);
		float dy = 0.1f*static_cast<float>(y - mLastMousePos.y);

		// 根据鼠标输入更新摄像机半径
		mRadius -= dx - dy;

		// 限制m_radius
		mRadius = MathHelper::Clamp(mRadius, 20.0f, 500.0f);
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}

void BlurDemo::UpdateWaves()
{
	
}

void BlurDemo::DrawWrapper()
{
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
	Effects::BasicFX->SetFogColor(Colors::Silver);
	Effects::BasicFX->SetFogStart(15.0f);
	Effects::BasicFX->SetFogRange(175.0f);

	ID3DX11EffectTechnique* boxTech = nullptr;
	ID3DX11EffectTechnique* landAndWavesTech = nullptr;

	switch (mRenderOptions)
	{
	case RenderOptions::Lighting:
		boxTech = Effects::BasicFX->Light3Tech;
		landAndWavesTech = Effects::BasicFX->Light3Tech;
		break;
	case RenderOptions::Textures:
		boxTech = Effects::BasicFX->Light3TexAlphaClipTech;
		landAndWavesTech = Effects::BasicFX->Light3TexTech;
		break;
	case RenderOptions::TexturesAndFog:
		boxTech = Effects::BasicFX->Light3TexAlphaClipFogTech;
		landAndWavesTech = Effects::BasicFX->Light3TexFogTech;
		break;
	}

	D3DX11_TECHNIQUE_DESC techDesc;

	//
	// 画透明剪切箱子
	// 

	boxTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		m_pImmediateContext->IASetVertexBuffers(0, 1, &mBoxVB, &stride, &offset);
		m_pImmediateContext->IASetIndexBuffer(mBoxIB, DXGI_FORMAT_R32_UINT, 0);

		// Set per object constants.
		XMMATRIX world = XMLoadFloat4x4(&mBoxWorld);
		XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
		XMMATRIX worldViewProj = world * view*proj;

		Effects::BasicFX->SetWorld(world);
		Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BasicFX->SetWorldViewProj(worldViewProj);
		Effects::BasicFX->SetTexTransform(XMMatrixIdentity());
		Effects::BasicFX->SetMaterial(mBoxMat);
		Effects::BasicFX->SetDiffuseMap(mCrateSRV);

		m_pImmediateContext->RSSetState(RenderStates::NoCullRS);
		boxTech->GetPassByIndex(p)->Apply(0, m_pImmediateContext);
		m_pImmediateContext->DrawIndexed(36, 0, 0);

		// 重置光栅化状态
		m_pImmediateContext->RSSetState(0);
	}

	//
	// 用纹理和雾绘制山丘和水（不需要alpha裁剪）。
	//

	landAndWavesTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		//
		// 画山丘
		//
		m_pImmediateContext->IASetVertexBuffers(0, 1, &mLandVB, &stride, &offset);
		m_pImmediateContext->IASetIndexBuffer(mLandIB, DXGI_FORMAT_R32_UINT, 0);

		// 设置对象的常量缓冲区
		XMMATRIX world = XMLoadFloat4x4(&mLandWorld);
		XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
		XMMATRIX worldViewProj = world * view*proj;

		Effects::BasicFX->SetWorld(world);
		Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BasicFX->SetWorldViewProj(worldViewProj);
		Effects::BasicFX->SetTexTransform(XMLoadFloat4x4(&mGrassTexTransform));
		Effects::BasicFX->SetMaterial(mLandMat);
		Effects::BasicFX->SetDiffuseMap(mGrassMapSRV);

		landAndWavesTech->GetPassByIndex(p)->Apply(0, m_pImmediateContext);
		m_pImmediateContext->DrawIndexed(mLandIndexCount, 0, 0);

		//
		// 画水
		//
		m_pImmediateContext->IASetVertexBuffers(0, 1, &mWavesVB, &stride, &offset);
		m_pImmediateContext->IASetIndexBuffer(mWavesIB, DXGI_FORMAT_R32_UINT, 0);

		// 设置对象的常量缓冲区
		world = XMLoadFloat4x4(&mWavesWorld);
		worldInvTranspose = MathHelper::InverseTranspose(world);
		worldViewProj = world * view*proj;

		Effects::BasicFX->SetWorld(world);
		Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BasicFX->SetWorldViewProj(worldViewProj);
		Effects::BasicFX->SetTexTransform(XMLoadFloat4x4(&mWaterTexTransform));
		Effects::BasicFX->SetMaterial(mWavesMat);
		Effects::BasicFX->SetDiffuseMap(mWavesMapSRV);

		m_pImmediateContext->OMSetBlendState(RenderStates::TransparentBS, blendFactor, 0xffffffff);
		landAndWavesTech->GetPassByIndex(p)->Apply(0, m_pImmediateContext);
		m_pImmediateContext->DrawIndexed(3 * mWaves.TriangleCount(), 0, 0);

		// 重置默认混合状态
		m_pImmediateContext->OMSetBlendState(0, blendFactor, 0xffffffff);
	}
}

void BlurDemo::DrawScreenQuad()
{
	m_pImmediateContext->IASetInputLayout(InputLayouts::Basic32);
	m_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	UINT stride = sizeof(Vertex::Basic32);
	UINT offset = 0;

	XMMATRIX identity = XMMatrixIdentity();

	ID3DX11EffectTechnique* texOnlyTech = Effects::BasicFX->Light0TexTech;
	D3DX11_TECHNIQUE_DESC techDesc;

	texOnlyTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		m_pImmediateContext->IASetVertexBuffers(0, 1, &mScreenQuadVB, &stride, &offset);
		m_pImmediateContext->IASetIndexBuffer(mScreenQuadIB, DXGI_FORMAT_R32_UINT, 0);

		Effects::BasicFX->SetWorld(identity);
		Effects::BasicFX->SetWorldInvTranspose(identity);
		Effects::BasicFX->SetWorldViewProj(identity);
		Effects::BasicFX->SetTexTransform(identity);
		Effects::BasicFX->SetDiffuseMap(mBlur.GetBlurredOutput());

		texOnlyTech->GetPassByIndex(p)->Apply(0, m_pImmediateContext);
		m_pImmediateContext->DrawIndexed(6, 0, 0);
	}
}

float BlurDemo::GetHillHeight(float x, float z) const
{
	return 0.3f*(z*sinf(0.1f*x) + x * cosf(0.1f*z));
}

XMFLOAT3 BlurDemo::GetHillNormal(float x, float z) const
{
	XMFLOAT3 n(
		-0.03f*z*cosf(0.1f*x) - 0.3f*cosf(0.1f*z),
		1.0f,
		-0.3f*sinf(0.1f*x) + 0.03f*x*sinf(0.1f*z));

	XMVECTOR unitNormal = XMVector3Normalize(XMLoadFloat3(&n));
	XMStoreFloat3(&n, unitNormal);

	return n;
}

void BlurDemo::BuildLandGeometryBuffers()
{
	GeometryGenerator::MeshData grid;

	GeometryGenerator geoGen;

	geoGen.CreateGrid(160.0f, 160.0f, 50, 50, grid);

	mLandIndexCount = grid.Indices.size();


	std::vector<Vertex::Basic32> vertices(grid.Vertices.size());
	for (UINT i = 0; i < grid.Vertices.size(); ++i)
	{
		XMFLOAT3 p = grid.Vertices[i].Position;

		p.y = GetHillHeight(p.x, p.z);

		vertices[i].Pos = p;
		vertices[i].Normal = GetHillNormal(p.x, p.z);
		vertices[i].Tex = grid.Vertices[i].TexC;
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex::Basic32) * grid.Vertices.size();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	HR(m_pD3dDevice->CreateBuffer(&vbd, &vinitData, &mLandVB));


	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * mLandIndexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &grid.Indices[0];
	HR(m_pD3dDevice->CreateBuffer(&ibd, &iinitData, &mLandIB));
}

void BlurDemo::BuildWaveGeometryBuffers()
{
	//创建水顶点缓冲区，注意我们只分配空间，在每个时间更新数据
	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_DYNAMIC;
	vbd.ByteWidth = sizeof(Vertex::Basic32) * mWaves.VertexCount();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vbd.MiscFlags = 0;
	HR(m_pD3dDevice->CreateBuffer(&vbd, 0, &mWavesVB));


	// 创建索引缓冲区。 索引缓冲区是固定的，所以我们只
	// 需要创建并设置一次。

	std::vector<UINT> indices(3 * mWaves.TriangleCount()); // 每个面3个索引

														   // 迭代每个四元数。
	UINT m = mWaves.RowCount();
	UINT n = mWaves.ColumnCount();
	int k = 0;
	for (UINT i = 0; i < m - 1; ++i)
	{
		for (DWORD j = 0; j < n - 1; ++j)
		{
			indices[k] = i * n + j;
			indices[k + 1] = i * n + j + 1;
			indices[k + 2] = (i + 1)*n + j;

			indices[k + 3] = (i + 1)*n + j;
			indices[k + 4] = i * n + j + 1;
			indices[k + 5] = (i + 1)*n + j + 1;

			k += 6; // 下一个四元数
		}
	}

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * indices.size();
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &indices[0];
	HR(m_pD3dDevice->CreateBuffer(&ibd, &iinitData, &mWavesIB));
}

void BlurDemo::BuildCrateGeometryBuffers()
{
	GeometryGenerator::MeshData box;

	GeometryGenerator geoGen;
	geoGen.CreateBox(1.0f, 1.0f, 1.0f, box);


	std::vector<Vertex::Basic32> vertices(box.Vertices.size());

	for (UINT i = 0; i < box.Vertices.size(); ++i)
	{
		vertices[i].Pos = box.Vertices[i].Position;
		vertices[i].Normal = box.Vertices[i].Normal;
		vertices[i].Tex = box.Vertices[i].TexC;
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex::Basic32) * box.Vertices.size();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	HR(m_pD3dDevice->CreateBuffer(&vbd, &vinitData, &mBoxVB));


	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * box.Indices.size();
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &box.Indices[0];
	HR(m_pD3dDevice->CreateBuffer(&ibd, &iinitData, &mBoxIB));
}

void BlurDemo::BuildScreenQuadGeometryBuffers()
{
	GeometryGenerator::MeshData quad;

	GeometryGenerator geoGen;
	geoGen.CreateFullscreenQuad(quad);


	std::vector<Vertex::Basic32> vertices(quad.Vertices.size());

	for (UINT i = 0; i < quad.Vertices.size(); ++i)
	{
		vertices[i].Pos = quad.Vertices[i].Position;
		vertices[i].Normal = quad.Vertices[i].Normal;
		vertices[i].Tex = quad.Vertices[i].TexC;
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex::Basic32) * quad.Vertices.size();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	HR(m_pD3dDevice->CreateBuffer(&vbd, &vinitData, &mScreenQuadVB));


	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * quad.Indices.size();
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &quad.Indices[0];
	HR(m_pD3dDevice->CreateBuffer(&ibd, &iinitData, &mScreenQuadIB));
}

void BlurDemo::BuildOffscreenViews()
{
	//每次窗口尺寸改变我们就调用此函数，重新创建离屏视图
	//创建之前先释放
	ReleaseCOM(mOffscreenSRV);
	ReleaseCOM(mOffscreenRTV);
	ReleaseCOM(mOffscreenUAV);

	D3D11_TEXTURE2D_DESC texDesc;

	texDesc.Width = m_clientWidth;
	texDesc.Height = m_clientHeight;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;

	ID3D11Texture2D* offscreenTex = 0;
	HR(m_pD3dDevice->CreateTexture2D(&texDesc, 0, &offscreenTex));

	//空描述意味着使用创建一个视图到所有mipmap级别
	//纹理创建的格式。
	HR(m_pD3dDevice->CreateShaderResourceView(offscreenTex, 0, &mOffscreenSRV));
	HR(m_pD3dDevice->CreateRenderTargetView(offscreenTex, 0, &mOffscreenRTV));
	HR(m_pD3dDevice->CreateUnorderedAccessView(offscreenTex, 0, &mOffscreenUAV));

	// 视图会保持纹理的引用，所以我们可以把纹理释放了。
	ReleaseCOM(offscreenTex);
}















