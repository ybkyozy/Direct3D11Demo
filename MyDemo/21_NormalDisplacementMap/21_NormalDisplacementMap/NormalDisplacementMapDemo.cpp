#include "NormalDisplacementMapDemo.h"

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	//上面是检测内存泄漏

	DynamicCubeMapDemo theApp(hInstance);

	if (!theApp.Init())
		return  0;
	return theApp.Run();
}

DynamicCubeMapDemo::DynamicCubeMapDemo(HINSTANCE hinstance):D3DApp(hinstance)
, mSky(0), mShapesVB(0), mShapesIB(0), mSkullVB(0), mSkullIB(0),
mStoneTexSRV(0), mBrickTexSRV(0),
mStoneNormalTexSRV(0), mBrickNormalTexSRV(0),
mSkullIndexCount(0), mRenderOptions(RenderOptionsNormalMap)
{
	m_mainWndCaption = L"D3D11 法线贴图 和 位移贴图";

	mLastMousePos.x = 0;
	mLastMousePos.y = 0;

	mCam.SetPosition(0.0f, 2.0f, -15.0f);

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&mGridWorld, I);

	XMMATRIX boxScale = XMMatrixScaling(3.0f, 1.0f, 3.0f);
	XMMATRIX boxOffset = XMMatrixTranslation(0.0f, 0.5f, 0.0f);
	XMStoreFloat4x4(&mBoxWorld, XMMatrixMultiply(boxScale, boxOffset));

	XMMATRIX skullScale = XMMatrixScaling(0.5f, 0.5f, 0.5f);
	XMMATRIX skullOffset = XMMatrixTranslation(0.0f, 1.0f, 0.0f);
	XMStoreFloat4x4(&mSkullWorld, XMMatrixMultiply(skullScale, skullOffset));

	for (int i = 0; i < 5; ++i)
	{
		XMStoreFloat4x4(&mCylWorld[i * 2 + 0], XMMatrixTranslation(-5.0f, 1.5f, -10.0f + i * 5.0f));
		XMStoreFloat4x4(&mCylWorld[i * 2 + 1], XMMatrixTranslation(+5.0f, 1.5f, -10.0f + i * 5.0f));

		XMStoreFloat4x4(&mSphereWorld[i * 2 + 0], XMMatrixTranslation(-5.0f, 3.5f, -10.0f + i * 5.0f));
		XMStoreFloat4x4(&mSphereWorld[i * 2 + 1], XMMatrixTranslation(+5.0f, 3.5f, -10.0f + i * 5.0f));
	}

	mDirLights[0].Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mDirLights[0].Diffuse = XMFLOAT4(0.7f, 0.7f, 0.6f, 1.0f);
	mDirLights[0].Specular = XMFLOAT4(0.8f, 0.8f, 0.7f, 1.0f);
	mDirLights[0].Direction = XMFLOAT3(0.707f, 0.0f, 0.707f);

	mDirLights[1].Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mDirLights[1].Diffuse = XMFLOAT4(0.40f, 0.40f, 0.40f, 1.0f);
	mDirLights[1].Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mDirLights[1].Direction = XMFLOAT3(0.0f, -0.707f, 0.707f);

	mDirLights[2].Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mDirLights[2].Diffuse = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mDirLights[2].Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mDirLights[2].Direction = XMFLOAT3(-0.57735f, -0.57735f, -0.57735f);

	mGridMat.Ambient = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	mGridMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mGridMat.Specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 16.0f);
	mGridMat.Reflect = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

	mCylinderMat.Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mCylinderMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mCylinderMat.Specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 32.0f);
	mCylinderMat.Reflect = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

	mSphereMat.Ambient = XMFLOAT4(0.2f, 0.3f, 0.4f, 1.0f);
	mSphereMat.Diffuse = XMFLOAT4(0.2f, 0.3f, 0.4f, 1.0f);
	mSphereMat.Specular = XMFLOAT4(0.9f, 0.9f, 0.9f, 16.0f);
	mSphereMat.Reflect = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);

	mBoxMat.Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mBoxMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mBoxMat.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 16.0f);
	mBoxMat.Reflect = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

	mSkullMat.Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mSkullMat.Diffuse = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mSkullMat.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 16.0f);
	mSkullMat.Reflect = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);

}


DynamicCubeMapDemo::~DynamicCubeMapDemo()
{
	SafeDelete(mSky);

	ReleaseCOM(mShapesVB);
	ReleaseCOM(mShapesIB);
	ReleaseCOM(mSkullVB);
	ReleaseCOM(mSkullIB);
	ReleaseCOM(mStoneTexSRV);
	ReleaseCOM(mBrickTexSRV);
	ReleaseCOM(mStoneNormalTexSRV);
	ReleaseCOM(mBrickNormalTexSRV);

	Effects::DestroyAll();
	InputLayouts::DestroyAll();
	RenderStates::DestroyAll();
}

bool DynamicCubeMapDemo::Init()
{
	if (!D3DApp::Init())
	{
		return false;
	}
	//必须先初始化效果，因为输入布局的初始化依赖它
	Effects::InitAll(m_pD3dDevice);
	InputLayouts::InitAll(m_pD3dDevice);
	RenderStates::InitAll(m_pD3dDevice);
	
	mSky = new Sky(m_pD3dDevice, L"Textures/snowcube1024.dds", 5000.0f);

	HR(CreateDDSTextureFromFile(m_pD3dDevice, L"Textures/floor.dds", 0, &mStoneTexSRV));

	HR(CreateDDSTextureFromFile(m_pD3dDevice, L"Textures/bricks.dds", 0, &mBrickTexSRV));

	HR(CreateDDSTextureFromFile(m_pD3dDevice, L"Textures/floor_nmap.dds", 0, &mStoneNormalTexSRV));

	HR(CreateDDSTextureFromFile(m_pD3dDevice, L"Textures/bricks_nmap.dds", 0, &mBrickNormalTexSRV));


	BuildShapeGeometryBuffers();
	BuildSkullGeometryBuffers();
	

	return true;
}

void DynamicCubeMapDemo::OnResize()
{
	D3DApp::OnResize();

	mCam.SetLens(0.25f*MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);

}

void DynamicCubeMapDemo::UpdateScene(float dt)
{
	//
	// 控制摄像机
	//
	if (GetAsyncKeyState('W') & 0x8000)
		mCam.Walk(10.0f*dt);

	if (GetAsyncKeyState('S') & 0x8000)
		mCam.Walk(-10.0f*dt);

	if (GetAsyncKeyState('A') & 0x8000)
		mCam.Strafe(-10.0f*dt);

	if (GetAsyncKeyState('D') & 0x8000)
		mCam.Strafe(10.0f*dt);

	//
	// 根据按键切换渲染效果。
	//
	if (GetAsyncKeyState('2') & 0x8000)
		mRenderOptions = RenderOptionsBasic;

	if (GetAsyncKeyState('3') & 0x8000)
		mRenderOptions = RenderOptionsNormalMap;

	if (GetAsyncKeyState('4') & 0x8000)
		mRenderOptions = RenderOptionsDisplacementMap;
}

void DynamicCubeMapDemo::DrawScene()
{
	m_pImmediateContext->ClearRenderTargetView(m_pRenderTargetView, reinterpret_cast<const float*>(&Colors::Silver));
	m_pImmediateContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	mCam.UpdateViewMatrix();

	XMMATRIX view = mCam.View();
	XMMATRIX proj = mCam.Proj();
	XMMATRIX viewProj = mCam.ViewProj();

	float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };

	// 设置每帧常量缓冲区
	Effects::BasicFX->SetDirLights(mDirLights);
	Effects::BasicFX->SetEyePosW(mCam.GetPosition());
	Effects::BasicFX->SetCubeMap(mSky->CubeMapSRV());

	Effects::NormalMapFX->SetDirLights(mDirLights);
	Effects::NormalMapFX->SetEyePosW(mCam.GetPosition());
	Effects::NormalMapFX->SetCubeMap(mSky->CubeMapSRV());

	Effects::DisplacementMapFX->SetDirLights(mDirLights);
	Effects::DisplacementMapFX->SetEyePosW(mCam.GetPosition());
	Effects::DisplacementMapFX->SetCubeMap(mSky->CubeMapSRV());

	// 如果需要，可以为每个对象设置这些属性。
	Effects::DisplacementMapFX->SetHeightScale(0.07f);
	Effects::DisplacementMapFX->SetMaxTessDistance(1.0f);
	Effects::DisplacementMapFX->SetMinTessDistance(25.0f);
	Effects::DisplacementMapFX->SetMinTessFactor(1.0f);
	Effects::DisplacementMapFX->SetMaxTessFactor(5.0f);

	// 找出哪种技术用于不同的几何体。

	ID3DX11EffectTechnique* activeTech = Effects::DisplacementMapFX->Light3TexTech;
	ID3DX11EffectTechnique* activeSphereTech = Effects::BasicFX->Light3ReflectTech;
	ID3DX11EffectTechnique* activeSkullTech = Effects::BasicFX->Light3ReflectTech;
	switch (mRenderOptions)
	{
	case RenderOptionsBasic:
		activeTech = Effects::BasicFX->Light3TexTech;
		m_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		break;
	case RenderOptionsNormalMap:
		activeTech = Effects::NormalMapFX->Light3TexTech;
		m_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		break;
	case RenderOptionsDisplacementMap:
		activeTech = Effects::DisplacementMapFX->Light3TexTech;
		m_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
		break;
	}

	XMMATRIX world;
	XMMATRIX worldInvTranspose;
	XMMATRIX worldViewProj;

	//
	// 绘制没有任何立方体反射的网格，圆柱体和盒子。
	// 

	UINT stride = sizeof(Vertex::PosNormalTexTan);
	UINT offset = 0;

	m_pImmediateContext->IASetInputLayout(InputLayouts::PosNormalTexTan);
	m_pImmediateContext->IASetVertexBuffers(0, 1, &mShapesVB, &stride, &offset);
	m_pImmediateContext->IASetIndexBuffer(mShapesIB, DXGI_FORMAT_R32_UINT, 0);

	if (GetAsyncKeyState('1') & 0x8000)
		m_pImmediateContext->RSSetState(RenderStates::WireframeRS);

	D3DX11_TECHNIQUE_DESC techDesc;
	activeTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		// 画网格
		world = XMLoadFloat4x4(&mGridWorld);
		worldInvTranspose = MathHelper::InverseTranspose(world);
		worldViewProj = world * view*proj;

		switch (mRenderOptions)
		{
		case RenderOptionsBasic:
			Effects::BasicFX->SetWorld(world);
			Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
			Effects::BasicFX->SetWorldViewProj(worldViewProj);
			Effects::BasicFX->SetTexTransform(XMMatrixScaling(8.0f, 10.0f, 1.0f));
			Effects::BasicFX->SetMaterial(mGridMat);
			Effects::BasicFX->SetDiffuseMap(mStoneTexSRV);
			break;
		case RenderOptionsNormalMap:
			Effects::NormalMapFX->SetWorld(world);
			Effects::NormalMapFX->SetWorldInvTranspose(worldInvTranspose);
			Effects::NormalMapFX->SetWorldViewProj(worldViewProj);
			Effects::NormalMapFX->SetTexTransform(XMMatrixScaling(8.0f, 10.0f, 1.0f));
			Effects::NormalMapFX->SetMaterial(mGridMat);
			Effects::NormalMapFX->SetDiffuseMap(mStoneTexSRV);
			Effects::NormalMapFX->SetNormalMap(mStoneNormalTexSRV);
			break;
		case RenderOptionsDisplacementMap:
			Effects::DisplacementMapFX->SetWorld(world);
			Effects::DisplacementMapFX->SetWorldInvTranspose(worldInvTranspose);
			Effects::DisplacementMapFX->SetViewProj(viewProj);
			Effects::DisplacementMapFX->SetWorldViewProj(worldViewProj);
			Effects::DisplacementMapFX->SetTexTransform(XMMatrixScaling(8.0f, 10.0f, 1.0f));
			Effects::DisplacementMapFX->SetMaterial(mGridMat);
			Effects::DisplacementMapFX->SetDiffuseMap(mStoneTexSRV);
			Effects::DisplacementMapFX->SetNormalMap(mStoneNormalTexSRV);
			break;
		}

		activeTech->GetPassByIndex(p)->Apply(0, m_pImmediateContext);
		m_pImmediateContext->DrawIndexed(mGridIndexCount, mGridIndexOffset, mGridVertexOffset);

		// 画盒子
		world = XMLoadFloat4x4(&mBoxWorld);
		worldInvTranspose = MathHelper::InverseTranspose(world);
		worldViewProj = world * view*proj;

		switch (mRenderOptions)
		{
		case RenderOptionsBasic:
			Effects::BasicFX->SetWorld(world);
			Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
			Effects::BasicFX->SetWorldViewProj(worldViewProj);
			Effects::BasicFX->SetTexTransform(XMMatrixScaling(2.0f, 1.0f, 1.0f));
			Effects::BasicFX->SetMaterial(mBoxMat);
			Effects::BasicFX->SetDiffuseMap(mBrickTexSRV);
			break;
		case RenderOptionsNormalMap:
			Effects::NormalMapFX->SetWorld(world);
			Effects::NormalMapFX->SetWorldInvTranspose(worldInvTranspose);
			Effects::NormalMapFX->SetWorldViewProj(worldViewProj);
			Effects::NormalMapFX->SetTexTransform(XMMatrixScaling(2.0f, 1.0f, 1.0f));
			Effects::NormalMapFX->SetMaterial(mBoxMat);
			Effects::NormalMapFX->SetDiffuseMap(mBrickTexSRV);
			Effects::NormalMapFX->SetNormalMap(mBrickNormalTexSRV);
			break;
		case RenderOptionsDisplacementMap:
			Effects::DisplacementMapFX->SetWorld(world);
			Effects::DisplacementMapFX->SetWorldInvTranspose(worldInvTranspose);
			Effects::DisplacementMapFX->SetViewProj(viewProj);
			Effects::DisplacementMapFX->SetWorldViewProj(worldViewProj);
			Effects::DisplacementMapFX->SetTexTransform(XMMatrixScaling(2.0f, 1.0f, 1.0f));
			Effects::DisplacementMapFX->SetMaterial(mBoxMat);
			Effects::DisplacementMapFX->SetDiffuseMap(mBrickTexSRV);
			Effects::DisplacementMapFX->SetNormalMap(mBrickNormalTexSRV);
			break;
		}

		activeTech->GetPassByIndex(p)->Apply(0, m_pImmediateContext);
		m_pImmediateContext->DrawIndexed(mBoxIndexCount, mBoxIndexOffset, mBoxVertexOffset);

		// 画圆柱
		for (int i = 0; i < 10; ++i)
		{
			world = XMLoadFloat4x4(&mCylWorld[i]);
			worldInvTranspose = MathHelper::InverseTranspose(world);
			worldViewProj = world * view*proj;

			switch (mRenderOptions)
			{
			case RenderOptionsBasic:
				Effects::BasicFX->SetWorld(world);
				Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
				Effects::BasicFX->SetWorldViewProj(worldViewProj);
				Effects::BasicFX->SetTexTransform(XMMatrixScaling(1.0f, 2.0f, 1.0f));
				Effects::BasicFX->SetMaterial(mCylinderMat);
				Effects::BasicFX->SetDiffuseMap(mBrickTexSRV);
				break;
			case RenderOptionsNormalMap:
				Effects::NormalMapFX->SetWorld(world);
				Effects::NormalMapFX->SetWorldInvTranspose(worldInvTranspose);
				Effects::NormalMapFX->SetWorldViewProj(worldViewProj);
				Effects::NormalMapFX->SetTexTransform(XMMatrixScaling(1.0f, 2.0f, 1.0f));
				Effects::NormalMapFX->SetMaterial(mCylinderMat);
				Effects::NormalMapFX->SetDiffuseMap(mBrickTexSRV);
				Effects::NormalMapFX->SetNormalMap(mBrickNormalTexSRV);
				break;
			case RenderOptionsDisplacementMap:
				Effects::DisplacementMapFX->SetWorld(world);
				Effects::DisplacementMapFX->SetWorldInvTranspose(worldInvTranspose);
				Effects::DisplacementMapFX->SetViewProj(viewProj);
				Effects::DisplacementMapFX->SetWorldViewProj(worldViewProj);
				Effects::DisplacementMapFX->SetTexTransform(XMMatrixScaling(1.0f, 2.0f, 1.0f));
				Effects::DisplacementMapFX->SetMaterial(mCylinderMat);
				Effects::DisplacementMapFX->SetDiffuseMap(mBrickTexSRV);
				Effects::DisplacementMapFX->SetNormalMap(mBrickNormalTexSRV);
				break;
			}


			activeTech->GetPassByIndex(p)->Apply(0, m_pImmediateContext);
			m_pImmediateContext->DrawIndexed(mCylinderIndexCount, mCylinderIndexOffset, mCylinderVertexOffset);
		}
	}

	// FX设置曲面细分阶段，但不会禁用它们。 所以在这里
	// 关闭曲面细分。
	m_pImmediateContext->HSSetShader(0, 0, 0);
	m_pImmediateContext->DSSetShader(0, 0, 0);

	m_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//
	// 用立方体反射贴图绘制球体。
	//

	activeSphereTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		// Draw the spheres.
		for (int i = 0; i < 10; ++i)
		{
			world = XMLoadFloat4x4(&mSphereWorld[i]);
			worldInvTranspose = MathHelper::InverseTranspose(world);
			worldViewProj = world * view*proj;

			Effects::BasicFX->SetWorld(world);
			Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
			Effects::BasicFX->SetWorldViewProj(worldViewProj);
			Effects::BasicFX->SetTexTransform(XMMatrixIdentity());
			Effects::BasicFX->SetMaterial(mSphereMat);

			activeSphereTech->GetPassByIndex(p)->Apply(0, m_pImmediateContext);
			m_pImmediateContext->DrawIndexed(mSphereIndexCount, mSphereIndexOffset, mSphereVertexOffset);
		}
	}

	stride = sizeof(Vertex::Basic32);
	offset = 0;

	m_pImmediateContext->RSSetState(0);

	m_pImmediateContext->IASetInputLayout(InputLayouts::Basic32);
	m_pImmediateContext->IASetVertexBuffers(0, 1, &mSkullVB, &stride, &offset);
	m_pImmediateContext->IASetIndexBuffer(mSkullIB, DXGI_FORMAT_R32_UINT, 0);

	activeSkullTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		// 画骷髅
		world = XMLoadFloat4x4(&mSkullWorld);
		worldInvTranspose = MathHelper::InverseTranspose(world);
		worldViewProj = world * view*proj;

		Effects::BasicFX->SetWorld(world);
		Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BasicFX->SetWorldViewProj(worldViewProj);
		Effects::BasicFX->SetMaterial(mSkullMat);

		activeSkullTech->GetPassByIndex(p)->Apply(0, m_pImmediateContext);
		m_pImmediateContext->DrawIndexed(mSkullIndexCount, 0, 0);
	}

	mSky->Draw(m_pImmediateContext, mCam);

	// 恢复默认状态，因为SkyFX在效果文件中更改它们。
	m_pImmediateContext->RSSetState(0);
	m_pImmediateContext->OMSetDepthStencilState(0, 0);

	HR(m_pSwapChain->Present(0, 0));
}

void DynamicCubeMapDemo::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	SetCapture(m_hMainWnd);
}

void DynamicCubeMapDemo::OnMouseUp(WPARAM btnState, int x, int y)
{
	// 不需要继续获得鼠标消息就释放掉
	ReleaseCapture();
}

void DynamicCubeMapDemo::OnMouseMove(WPARAM btnState, int x, int y)
{
	
	// 鼠标左键调整视角角度
	if ((btnState & MK_LBUTTON) != 0)
	{
		// 根据鼠标Pos和lastPos在x/y方向上的变化量dx/dy得出角度，每个像素的距离相当于1度，再将角度转为弧度
		float dx = XMConvertToRadians(0.25f*static_cast<float>(x - mLastMousePos.x));
		float dy = XMConvertToRadians(0.25f*static_cast<float>(y - mLastMousePos.y));

		mCam.Pitch(dy);
		mCam.RotateY(dx);
	}
	// 鼠标左键调整视角距离
	else if ((btnState & MK_RBUTTON) != 0)
	{
		// 使每个像素对应于场景中的0.005个单元
		float dx = 0.05f*static_cast<float>(x - mLastMousePos.x);
		float dy = 0.05f*static_cast<float>(y - mLastMousePos.y);

		mCam.Strafe(dx);
		mCam.fly(-dy);
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}


void DynamicCubeMapDemo::BuildShapeGeometryBuffers()
{
	GeometryGenerator::MeshData box;
	GeometryGenerator::MeshData grid;
	GeometryGenerator::MeshData sphere;
	GeometryGenerator::MeshData cylinder;

	GeometryGenerator geoGen;
	geoGen.CreateBox(1.0f, 1.0f, 1.0f, box);
	geoGen.CreateGrid(20.0f, 30.0f, 50, 40, grid);
	geoGen.CreateSphere(0.5f, 20, 20, sphere);
	geoGen.CreateCylinder(0.5f, 0.5f, 3.0f, 15, 15, cylinder);

	// 缓存连接顶点缓冲区中每个对象的顶点偏移。
	mBoxVertexOffset = 0;
	mGridVertexOffset = box.Vertices.size();
	mSphereVertexOffset = mGridVertexOffset + grid.Vertices.size();
	mCylinderVertexOffset = mSphereVertexOffset + sphere.Vertices.size();

	// 缓存每个对象的索引数。
	mBoxIndexCount = box.Indices.size();
	mGridIndexCount = grid.Indices.size();
	mSphereIndexCount = sphere.Indices.size();
	mCylinderIndexCount = cylinder.Indices.size();

	// 缓存每个对象的索引偏移。
	mBoxIndexOffset = 0;
	mGridIndexOffset = mBoxIndexCount;
	mSphereIndexOffset = mGridIndexOffset + mGridIndexCount;
	mCylinderIndexOffset = mSphereIndexOffset + mSphereIndexCount;

	UINT totalVertexCount =
		box.Vertices.size() +
		grid.Vertices.size() +
		sphere.Vertices.size() +
		cylinder.Vertices.size();

	UINT totalIndexCount =
		mBoxIndexCount +
		mGridIndexCount +
		mSphereIndexCount +
		mCylinderIndexCount;

	//
	// 提取我们感兴趣的顶点元素并打包
	// 所有网格的顶点放入一个顶点缓冲区。
	//

	std::vector<Vertex::PosNormalTexTan> vertices(totalVertexCount);

	UINT k = 0;
	for (size_t i = 0; i < box.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = box.Vertices[i].Position;
		vertices[k].Normal = box.Vertices[i].Normal;
		vertices[k].Tex = box.Vertices[i].TexC;
		vertices[k].TangentU = box.Vertices[i].TangentU;
	}

	for (size_t i = 0; i < grid.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = grid.Vertices[i].Position;
		vertices[k].Normal = grid.Vertices[i].Normal;
		vertices[k].Tex = grid.Vertices[i].TexC;
		vertices[k].TangentU = grid.Vertices[i].TangentU;
	}

	for (size_t i = 0; i < sphere.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = sphere.Vertices[i].Position;
		vertices[k].Normal = sphere.Vertices[i].Normal;
		vertices[k].Tex = sphere.Vertices[i].TexC;
		vertices[k].TangentU = sphere.Vertices[i].TangentU;
	}

	for (size_t i = 0; i < cylinder.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = cylinder.Vertices[i].Position;
		vertices[k].Normal = cylinder.Vertices[i].Normal;
		vertices[k].Tex = cylinder.Vertices[i].TexC;
		vertices[k].TangentU = cylinder.Vertices[i].TangentU;
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex::PosNormalTexTan) * totalVertexCount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	HR(m_pD3dDevice->CreateBuffer(&vbd, &vinitData, &mShapesVB));

	//
	// 将所有网格的索引打包到一个索引缓冲区中。
	//

	std::vector<UINT> indices;
	indices.insert(indices.end(), box.Indices.begin(), box.Indices.end());
	indices.insert(indices.end(), grid.Indices.begin(), grid.Indices.end());
	indices.insert(indices.end(), sphere.Indices.begin(), sphere.Indices.end());
	indices.insert(indices.end(), cylinder.Indices.begin(), cylinder.Indices.end());

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * totalIndexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &indices[0];
	HR(m_pD3dDevice->CreateBuffer(&ibd, &iinitData, &mShapesIB));
}

void DynamicCubeMapDemo::BuildSkullGeometryBuffers()
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

	//
	// 将所有网格的索引打包到一个索引缓冲区中。
	//

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





















