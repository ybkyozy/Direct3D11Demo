#include "MeshViewDemo.h"

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	//上面是检测内存泄漏

	MeshViewDemo theApp(hInstance);

	if (!theApp.Init())
		return  0;
	return theApp.Run();
}

MeshViewDemo::MeshViewDemo(HINSTANCE hinstance):D3DApp(hinstance)
, mSky(0), mTreeModel(0), mBaseModel(0), mStairsModel(0),
mPillar1Model(0), mPillar2Model(0), mPillar3Model(0), mPillar4Model(0), mRockModel(0),
mScreenQuadVB(0), mScreenQuadIB(0),
mSmap(0), mSsao(0),
mLightRotationAngle(0.0f)
{
	m_mainWndCaption = L"D3D11 网格视图";

	mLastMousePos.x = 0;
	mLastMousePos.y = 0;

	mCam.SetPosition(0.0f, 2.0f, -15.0f);

	mDirLights[0].Ambient = XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f);
	mDirLights[0].Diffuse = XMFLOAT4(0.8f, 0.7f, 0.7f, 1.0f);
	mDirLights[0].Specular = XMFLOAT4(0.6f, 0.6f, 0.7f, 1.0f);
	mDirLights[0].Direction = XMFLOAT3(-0.57735f, -0.57735f, 0.57735f);

	mDirLights[1].Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mDirLights[1].Diffuse = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
	mDirLights[1].Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mDirLights[1].Direction = XMFLOAT3(0.707f, -0.707f, 0.0f);

	mDirLights[2].Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mDirLights[2].Diffuse = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	mDirLights[2].Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mDirLights[2].Direction = XMFLOAT3(0.0f, 0.0, -1.0f);

	mOriginalLightDir[0] = mDirLights[0].Direction;
	mOriginalLightDir[1] = mDirLights[1].Direction;
	mOriginalLightDir[2] = mDirLights[2].Direction;

}


MeshViewDemo::~MeshViewDemo()
{
	SafeDelete(mTreeModel);
	SafeDelete(mBaseModel);
	SafeDelete(mStairsModel);
	SafeDelete(mPillar1Model);
	SafeDelete(mPillar2Model);
	SafeDelete(mPillar3Model);
	SafeDelete(mPillar4Model);
	SafeDelete(mRockModel);

	SafeDelete(mSky);
	SafeDelete(mSmap);
	SafeDelete(mSsao);

	ReleaseCOM(mScreenQuadVB);
	ReleaseCOM(mScreenQuadIB);

	Effects::DestroyAll();
	InputLayouts::DestroyAll();
	RenderStates::DestroyAll();
}

bool MeshViewDemo::Init()
{
	if (!D3DApp::Init())
	{
		return false;
	}
	//必须先初始化效果，因为输入布局的初始化依赖它
	Effects::InitAll(m_pD3dDevice);
	InputLayouts::InitAll(m_pD3dDevice);
	RenderStates::InitAll(m_pD3dDevice);

	mTexMgr.Init(m_pD3dDevice);

	mSky = new Sky(m_pD3dDevice, L"Textures/desertcube1024.dds", 5000.0f);

	mSmap = new ShadowMap(m_pD3dDevice, SMapSize, SMapSize);

	mCam.SetLens(0.25f*MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	mSsao = new Ssao(m_pD3dDevice, m_pImmediateContext, m_clientWidth, m_clientHeight, mCam.GetFovY(), mCam.GetFarZ());

	BuildScreenQuadGeometryBuffers();

	mTreeModel = new BasicModel(m_pD3dDevice, mTexMgr, "Models\\tree.m3d", L"Textures\\");
	mBaseModel = new BasicModel(m_pD3dDevice, mTexMgr, "Models\\base.m3d", L"Textures\\");
	mStairsModel = new BasicModel(m_pD3dDevice, mTexMgr, "Models\\stairs.m3d", L"Textures\\");
	mPillar1Model = new BasicModel(m_pD3dDevice, mTexMgr, "Models\\pillar1.m3d", L"Textures\\");
	mPillar2Model = new BasicModel(m_pD3dDevice, mTexMgr, "Models\\pillar2.m3d", L"Textures\\");
	mPillar3Model = new BasicModel(m_pD3dDevice, mTexMgr, "Models\\pillar5.m3d", L"Textures\\");
	mPillar4Model = new BasicModel(m_pD3dDevice, mTexMgr, "Models\\pillar6.m3d", L"Textures\\");
	mRockModel = new BasicModel(m_pD3dDevice, mTexMgr, "Models\\rock.m3d", L"Textures\\");

	BasicModelInstance treeInstance;
	BasicModelInstance baseInstance;
	BasicModelInstance stairsInstance;
	BasicModelInstance pillar1Instance;
	BasicModelInstance pillar2Instance;
	BasicModelInstance pillar3Instance;
	BasicModelInstance pillar4Instance;
	BasicModelInstance rockInstance1;
	BasicModelInstance rockInstance2;
	BasicModelInstance rockInstance3;

	treeInstance.Model = mTreeModel;
	baseInstance.Model = mBaseModel;
	stairsInstance.Model = mStairsModel;
	pillar1Instance.Model = mPillar1Model;
	pillar2Instance.Model = mPillar2Model;
	pillar3Instance.Model = mPillar3Model;
	pillar4Instance.Model = mPillar4Model;
	rockInstance1.Model = mRockModel;
	rockInstance2.Model = mRockModel;
	rockInstance3.Model = mRockModel;

	XMMATRIX modelScale = XMMatrixScaling(1.0f, 1.0f, 1.0f);
	XMMATRIX modelRot = XMMatrixRotationY(0.0f);
	XMMATRIX modelOffset = XMMatrixTranslation(0.0f, 0.0f, 0.0f);
	XMStoreFloat4x4(&treeInstance.World, modelScale*modelRot*modelOffset);
	XMStoreFloat4x4(&baseInstance.World, modelScale*modelRot*modelOffset);

	modelRot = XMMatrixRotationY(0.5f*XM_PI);
	modelOffset = XMMatrixTranslation(0.0f, -2.5f, -12.0f);
	XMStoreFloat4x4(&stairsInstance.World, modelScale*modelRot*modelOffset);

	modelScale = XMMatrixScaling(0.8f, 0.8f, 0.8f);
	modelOffset = XMMatrixTranslation(-5.0f, 1.5f, 5.0f);
	XMStoreFloat4x4(&pillar1Instance.World, modelScale*modelRot*modelOffset);

	modelScale = XMMatrixScaling(0.8f, 0.8f, 0.8f);
	modelOffset = XMMatrixTranslation(5.0f, 1.5f, 5.0f);
	XMStoreFloat4x4(&pillar2Instance.World, modelScale*modelRot*modelOffset);

	modelScale = XMMatrixScaling(0.8f, 0.8f, 0.8f);
	modelOffset = XMMatrixTranslation(5.0f, 1.5f, -5.0f);
	XMStoreFloat4x4(&pillar3Instance.World, modelScale*modelRot*modelOffset);

	modelScale = XMMatrixScaling(1.0f, 1.0f, 1.0f);
	modelOffset = XMMatrixTranslation(-5.0f, 1.0f, -5.0f);
	XMStoreFloat4x4(&pillar4Instance.World, modelScale*modelRot*modelOffset);

	modelScale = XMMatrixScaling(0.8f, 0.8f, 0.8f);
	modelOffset = XMMatrixTranslation(-1.0f, 1.4f, -7.0f);
	XMStoreFloat4x4(&rockInstance1.World, modelScale*modelRot*modelOffset);

	modelScale = XMMatrixScaling(0.8f, 0.8f, 0.8f);
	modelOffset = XMMatrixTranslation(5.0f, 1.2f, -2.0f);
	XMStoreFloat4x4(&rockInstance2.World, modelScale*modelRot*modelOffset);

	modelScale = XMMatrixScaling(0.8f, 0.8f, 0.8f);
	modelOffset = XMMatrixTranslation(-4.0f, 1.3f, 3.0f);
	XMStoreFloat4x4(&rockInstance3.World, modelScale*modelRot*modelOffset);

	mAlphaClippedModelInstances.push_back(treeInstance);

	mModelInstances.push_back(baseInstance);
	mModelInstances.push_back(stairsInstance);
	mModelInstances.push_back(pillar1Instance);
	mModelInstances.push_back(pillar2Instance);
	mModelInstances.push_back(pillar3Instance);
	mModelInstances.push_back(pillar4Instance);
	mModelInstances.push_back(rockInstance1);
	mModelInstances.push_back(rockInstance2);
	mModelInstances.push_back(rockInstance3);

	//
	// Compute scene bounding box.
	//

	XMFLOAT3 minPt(+MathHelper::Infinity, +MathHelper::Infinity, +MathHelper::Infinity);
	XMFLOAT3 maxPt(-MathHelper::Infinity, -MathHelper::Infinity, -MathHelper::Infinity);
	for (UINT i = 0; i < mModelInstances.size(); ++i)
	{
		for (UINT j = 0; j < mModelInstances[i].Model->Vertices.size(); ++j)
		{
			XMFLOAT3 P = mModelInstances[i].Model->Vertices[j].Pos;

			minPt.x = MathHelper::Min(minPt.x, P.x);
			minPt.y = MathHelper::Min(minPt.x, P.x);
			minPt.z = MathHelper::Min(minPt.x, P.x);

			maxPt.x = MathHelper::Max(maxPt.x, P.x);
			maxPt.y = MathHelper::Max(maxPt.x, P.x);
			maxPt.z = MathHelper::Max(maxPt.x, P.x);
		}
	}

	//
	// Derive scene bounding sphere from bounding box.
	//
	mSceneBounds.Center = XMFLOAT3(
		0.5f*(minPt.x + maxPt.x),
		0.5f*(minPt.y + maxPt.y),
		0.5f*(minPt.z + maxPt.z));

	XMFLOAT3 extent(
		0.5f*(maxPt.x - minPt.x),
		0.5f*(maxPt.y - minPt.y),
		0.5f*(maxPt.z - minPt.z));

	mSceneBounds.Radius = sqrtf(extent.x*extent.x + extent.y*extent.y + extent.z*extent.z);


	return true;
}

void MeshViewDemo::OnResize()
{
	D3DApp::OnResize();

	mCam.SetLens(0.25f*MathHelper::Pi, AspectRatio(), 1.0f, 3000.0f);

	if (mSsao)
	{
		mSsao->OnSize(m_clientWidth, m_clientHeight, mCam.GetFovY(), mCam.GetFarZ());
	}
}

void MeshViewDemo::UpdateScene(float dt)
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

	BuildShadowTransform();

	mCam.UpdateViewMatrix();
}

void MeshViewDemo::DrawScene()
{
	//
	// Render the scene to the shadow map.
	//

	mSmap->BindDsvAndSetNullRenderTarget(m_pImmediateContext);

	DrawSceneToShadowMap();

	m_pImmediateContext->RSSetState(0);

	//
	// Render the view space normals and depths.  This render target has the
	// same dimensions as the back buffer, so we can use the screen viewport.
	// This render pass is needed to compute the ambient occlusion.
	// Notice that we use the main depth/stencil buffer in this pass.  
	//

	m_pImmediateContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	m_pImmediateContext->RSSetViewports(1, &m_screenViewPort);
	mSsao->SetNormalDepthRenderTarget(m_pDepthStencilView);

	DrawSceneToSsaoNormalDepthMap();

	//
	// Now compute the ambient occlusion.
	//

	mSsao->ComputeSsao(mCam);
	mSsao->BlurAmbientMap(2);

	//
	// Restore the back and depth buffer and viewport to the OM stage.
	//
	ID3D11RenderTargetView* renderTargets[1] = { m_pRenderTargetView };
	m_pImmediateContext->OMSetRenderTargets(1, renderTargets, m_pDepthStencilView);
	m_pImmediateContext->RSSetViewports(1, &m_screenViewPort);

	m_pImmediateContext->ClearRenderTargetView(m_pRenderTargetView, reinterpret_cast<const float*>(&Colors::Silver));

	// We already laid down scene depth to the depth buffer in the Normal/Depth map pass,
	// so we can set the depth comparison test to 揈QUALS.? This prevents any overdraw
	// in this rendering pass, as only the nearest visible pixels will pass this depth
	// comparison test.

	m_pImmediateContext->OMSetDepthStencilState(RenderStates::EqualsDSS, 0);

	XMMATRIX view = mCam.View();
	XMMATRIX proj = mCam.Proj();
	XMMATRIX viewProj = mCam.ViewProj();

	float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };

	// Set per frame constants.
	Effects::NormalMapFX->SetDirLights(mDirLights);
	Effects::NormalMapFX->SetEyePosW(mCam.GetPosition());
	Effects::NormalMapFX->SetCubeMap(mSky->CubeMapSRV());
	Effects::NormalMapFX->SetShadowMap(mSmap->DepthMapSRV());
	Effects::NormalMapFX->SetSsaoMap(mSsao->AmbientSRV());

	ID3DX11EffectTechnique* tech = Effects::NormalMapFX->Light3TexTech;
	ID3DX11EffectTechnique* alphaClippedTech = Effects::NormalMapFX->Light3TexAlphaClipTech;

	m_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


	XMMATRIX world;
	XMMATRIX worldInvTranspose;
	XMMATRIX worldViewProj;

	// Transform NDC space [-1,+1]^2 to texture space [0,1]^2
	XMMATRIX toTexSpace(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);

	XMMATRIX shadowTransform = XMLoadFloat4x4(&mShadowTransform);


	UINT stride = sizeof(Vertex::PosNormalTexTan);
	UINT offset = 0;

	m_pImmediateContext->IASetInputLayout(InputLayouts::PosNormalTexTan);

	if (GetAsyncKeyState('1') & 0x8000)
		m_pImmediateContext->RSSetState(RenderStates::WireframeRS);

	//
	// Draw opaque objects.
	//
	D3DX11_TECHNIQUE_DESC techDesc;
	tech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		for (UINT modelIndex = 0; modelIndex < mModelInstances.size(); ++modelIndex)
		{
			world = XMLoadFloat4x4(&mModelInstances[modelIndex].World);
			worldInvTranspose = MathHelper::InverseTranspose(world);
			worldViewProj = world * view*proj;

			Effects::NormalMapFX->SetWorld(world);
			Effects::NormalMapFX->SetWorldInvTranspose(worldInvTranspose);
			Effects::NormalMapFX->SetWorldViewProj(worldViewProj);
			Effects::NormalMapFX->SetWorldViewProjTex(worldViewProj*toTexSpace);
			Effects::NormalMapFX->SetShadowTransform(world*shadowTransform);
			Effects::NormalMapFX->SetTexTransform(XMMatrixScaling(1.0f, 1.0f, 1.0f));

			for (UINT subset = 0; subset < mModelInstances[modelIndex].Model->SubsetCount; ++subset)
			{
				Effects::NormalMapFX->SetMaterial(mModelInstances[modelIndex].Model->Mat[subset]);
				Effects::NormalMapFX->SetDiffuseMap(mModelInstances[modelIndex].Model->DiffuseMapSRV[subset]);
				Effects::NormalMapFX->SetNormalMap(mModelInstances[modelIndex].Model->NormalMapSRV[subset]);

				tech->GetPassByIndex(p)->Apply(0, m_pImmediateContext);
				mModelInstances[modelIndex].Model->ModelMesh.Draw(m_pImmediateContext, subset);
			}
		}
	}

	// The alpha tested triangles are leaves, so render them double sided.
	m_pImmediateContext->RSSetState(RenderStates::NoCullRS);
	alphaClippedTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		for (UINT modelIndex = 0; modelIndex < mAlphaClippedModelInstances.size(); ++modelIndex)
		{
			world = XMLoadFloat4x4(&mAlphaClippedModelInstances[modelIndex].World);
			worldInvTranspose = MathHelper::InverseTranspose(world);
			worldViewProj = world * view*proj;

			Effects::NormalMapFX->SetWorld(world);
			Effects::NormalMapFX->SetWorldInvTranspose(worldInvTranspose);
			Effects::NormalMapFX->SetWorldViewProj(worldViewProj);
			Effects::NormalMapFX->SetWorldViewProjTex(worldViewProj*toTexSpace);
			Effects::NormalMapFX->SetShadowTransform(world*shadowTransform);
			Effects::NormalMapFX->SetTexTransform(XMMatrixScaling(1.0f, 1.0f, 1.0f));

			for (UINT subset = 0; subset < mAlphaClippedModelInstances[modelIndex].Model->SubsetCount; ++subset)
			{
				Effects::NormalMapFX->SetMaterial(mAlphaClippedModelInstances[modelIndex].Model->Mat[subset]);
				Effects::NormalMapFX->SetDiffuseMap(mAlphaClippedModelInstances[modelIndex].Model->DiffuseMapSRV[subset]);
				Effects::NormalMapFX->SetNormalMap(mAlphaClippedModelInstances[modelIndex].Model->NormalMapSRV[subset]);

				alphaClippedTech->GetPassByIndex(p)->Apply(0, m_pImmediateContext);
				mAlphaClippedModelInstances[modelIndex].Model->ModelMesh.Draw(m_pImmediateContext, subset);
			}
		}
	}

	// Turn off wireframe.
	m_pImmediateContext->RSSetState(0);

	// Restore from RenderStates::EqualsDSS
	m_pImmediateContext->OMSetDepthStencilState(0, 0);

	// Debug view SSAO map.
	if (GetAsyncKeyState('Z') & 0x8000)
	{
		DrawScreenQuad(mSsao->AmbientSRV());
	}

	mSky->Draw(m_pImmediateContext, mCam);

	// restore default states, as the SkyFX changes them in the effect file.
	m_pImmediateContext->RSSetState(0);
	m_pImmediateContext->OMSetDepthStencilState(0, 0);

	// Unbind shadow map and AmbientMap as a shader input because we are going to render
	// to it next frame.  These textures can be at any slot, so clear all slots.
	ID3D11ShaderResourceView* nullSRV[16] = { 0 };
	m_pImmediateContext->PSSetShaderResources(0, 16, nullSRV);


	HR(m_pSwapChain->Present(0, 0));
}

void MeshViewDemo::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	SetCapture(m_hMainWnd);
}

void MeshViewDemo::OnMouseUp(WPARAM btnState, int x, int y)
{
	// 不需要继续获得鼠标消息就释放掉
	ReleaseCapture();
}

void MeshViewDemo::OnMouseMove(WPARAM btnState, int x, int y)
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

void MeshViewDemo::DrawSceneToSsaoNormalDepthMap()
{
	XMMATRIX view = mCam.View();
	XMMATRIX proj = mCam.Proj();
	XMMATRIX viewProj = XMMatrixMultiply(view, proj);

	ID3DX11EffectTechnique* tech = Effects::SsaoNormalDepthFX->NormalDepthTech;
	ID3DX11EffectTechnique* alphaClippedTech = Effects::SsaoNormalDepthFX->NormalDepthAlphaClipTech;

	XMMATRIX world;
	XMMATRIX worldInvTranspose;
	XMMATRIX worldView;
	XMMATRIX worldInvTransposeView;
	XMMATRIX worldViewProj;


	m_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_pImmediateContext->IASetInputLayout(InputLayouts::PosNormalTexTan);

	if (GetAsyncKeyState('1') & 0x8000)
		m_pImmediateContext->RSSetState(RenderStates::WireframeRS);

	D3DX11_TECHNIQUE_DESC techDesc;
	tech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		for (UINT modelIndex = 0; modelIndex < mModelInstances.size(); ++modelIndex)
		{
			world = XMLoadFloat4x4(&mModelInstances[modelIndex].World);
			worldInvTranspose = MathHelper::InverseTranspose(world);
			worldView = world * view;
			worldInvTransposeView = worldInvTranspose * view;
			worldViewProj = world * view*proj;

			Effects::SsaoNormalDepthFX->SetWorldView(worldView);
			Effects::SsaoNormalDepthFX->SetWorldInvTransposeView(worldInvTransposeView);
			Effects::SsaoNormalDepthFX->SetWorldViewProj(worldViewProj);
			Effects::SsaoNormalDepthFX->SetTexTransform(XMMatrixScaling(1.0f, 1.0f, 1.0f));

			tech->GetPassByIndex(p)->Apply(0, m_pImmediateContext);
			for (UINT subset = 0; subset < mModelInstances[modelIndex].Model->SubsetCount; ++subset)
			{
				mModelInstances[modelIndex].Model->ModelMesh.Draw(m_pImmediateContext, subset);
			}
		}
	}

	// The alpha tested triangles are leaves, so render them double sided.
	m_pImmediateContext->RSSetState(RenderStates::NoCullRS);
	alphaClippedTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		for (UINT modelIndex = 0; modelIndex < mAlphaClippedModelInstances.size(); ++modelIndex)
		{
			world = XMLoadFloat4x4(&mAlphaClippedModelInstances[modelIndex].World);
			worldInvTranspose = MathHelper::InverseTranspose(world);
			worldView = world * view;
			worldInvTransposeView = worldInvTranspose * view;
			worldViewProj = world * view*proj;

			Effects::SsaoNormalDepthFX->SetWorldView(worldView);
			Effects::SsaoNormalDepthFX->SetWorldInvTransposeView(worldInvTransposeView);
			Effects::SsaoNormalDepthFX->SetWorldViewProj(worldViewProj);
			Effects::SsaoNormalDepthFX->SetTexTransform(XMMatrixScaling(1.0f, 1.0f, 1.0f));

			for (UINT subset = 0; subset < mAlphaClippedModelInstances[modelIndex].Model->SubsetCount; ++subset)
			{
				Effects::SsaoNormalDepthFX->SetDiffuseMap(mAlphaClippedModelInstances[modelIndex].Model->DiffuseMapSRV[subset]);
				alphaClippedTech->GetPassByIndex(p)->Apply(0, m_pImmediateContext);
				mAlphaClippedModelInstances[modelIndex].Model->ModelMesh.Draw(m_pImmediateContext, subset);
			}
		}
	}

	m_pImmediateContext->RSSetState(0);
}

void MeshViewDemo::DrawSceneToShadowMap()
{
	XMMATRIX view = XMLoadFloat4x4(&mLightView);
	XMMATRIX proj = XMLoadFloat4x4(&mLightProj);
	XMMATRIX viewProj = XMMatrixMultiply(view, proj);

	Effects::BuildShadowMapFX->SetEyePosW(mCam.GetPosition());
	Effects::BuildShadowMapFX->SetViewProj(viewProj);

	ID3DX11EffectTechnique* tech = Effects::BuildShadowMapFX->BuildShadowMapTech;
	ID3DX11EffectTechnique* alphaClippedTech = Effects::BuildShadowMapFX->BuildShadowMapAlphaClipTech;

	m_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	XMMATRIX world;
	XMMATRIX worldInvTranspose;
	XMMATRIX worldViewProj;

	m_pImmediateContext->IASetInputLayout(InputLayouts::PosNormalTexTan);

	if (GetAsyncKeyState('1') & 0x8000)
		m_pImmediateContext->RSSetState(RenderStates::WireframeRS);

	D3DX11_TECHNIQUE_DESC techDesc;
	tech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		for (UINT modelIndex = 0; modelIndex < mModelInstances.size(); ++modelIndex)
		{
			world = XMLoadFloat4x4(&mModelInstances[modelIndex].World);
			worldInvTranspose = MathHelper::InverseTranspose(world);
			worldViewProj = world * view*proj;

			Effects::BuildShadowMapFX->SetWorld(world);
			Effects::BuildShadowMapFX->SetWorldInvTranspose(worldInvTranspose);
			Effects::BuildShadowMapFX->SetWorldViewProj(worldViewProj);
			Effects::BuildShadowMapFX->SetTexTransform(XMMatrixScaling(1.0f, 1.0f, 1.0f));

			tech->GetPassByIndex(p)->Apply(0, m_pImmediateContext);

			for (UINT subset = 0; subset < mModelInstances[modelIndex].Model->SubsetCount; ++subset)
			{
				mModelInstances[modelIndex].Model->ModelMesh.Draw(m_pImmediateContext, subset);
			}
		}
	}

	alphaClippedTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		for (UINT modelIndex = 0; modelIndex < mAlphaClippedModelInstances.size(); ++modelIndex)
		{
			world = XMLoadFloat4x4(&mAlphaClippedModelInstances[modelIndex].World);
			worldInvTranspose = MathHelper::InverseTranspose(world);
			worldViewProj = world * view*proj;

			Effects::BuildShadowMapFX->SetWorld(world);
			Effects::BuildShadowMapFX->SetWorldInvTranspose(worldInvTranspose);
			Effects::BuildShadowMapFX->SetWorldViewProj(worldViewProj);
			Effects::BuildShadowMapFX->SetTexTransform(XMMatrixScaling(1.0f, 1.0f, 1.0f));

			for (UINT subset = 0; subset < mAlphaClippedModelInstances[modelIndex].Model->SubsetCount; ++subset)
			{
				Effects::BuildShadowMapFX->SetDiffuseMap(mAlphaClippedModelInstances[modelIndex].Model->DiffuseMapSRV[subset]);
				alphaClippedTech->GetPassByIndex(p)->Apply(0, m_pImmediateContext);
				mAlphaClippedModelInstances[modelIndex].Model->ModelMesh.Draw(m_pImmediateContext, subset);
			}
		}
	}

	m_pImmediateContext->RSSetState(0);
}

void MeshViewDemo::DrawScreenQuad(ID3D11ShaderResourceView* srv)
{
	UINT stride = sizeof(Vertex::Basic32);
	UINT offset = 0;

	m_pImmediateContext->IASetInputLayout(InputLayouts::Basic32);
	m_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_pImmediateContext->IASetVertexBuffers(0, 1, &mScreenQuadVB, &stride, &offset);
	m_pImmediateContext->IASetIndexBuffer(mScreenQuadIB, DXGI_FORMAT_R32_UINT, 0);

	// Scale and shift quad to lower-right corner.
	XMMATRIX world(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, -0.5f, 0.0f, 1.0f);

	ID3DX11EffectTechnique* tech = Effects::DebugTexFX->ViewRedTech;
	D3DX11_TECHNIQUE_DESC techDesc;

	tech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		Effects::DebugTexFX->SetWorldViewProj(world);
		Effects::DebugTexFX->SetTexture(srv);

		tech->GetPassByIndex(p)->Apply(0, m_pImmediateContext);
		m_pImmediateContext->DrawIndexed(6, 0, 0);
	}
}

void MeshViewDemo::BuildShadowTransform()
{
	// Only the first "main" light casts a shadow.
	XMVECTOR lightDir = XMLoadFloat3(&mDirLights[0].Direction);
	XMVECTOR lightPos = -2.0f*mSceneBounds.Radius*lightDir;
	XMVECTOR targetPos = XMLoadFloat3(&mSceneBounds.Center);
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX V = XMMatrixLookAtLH(lightPos, targetPos, up);

	// Transform bounding sphere to light space.
	XMFLOAT3 sphereCenterLS;
	XMStoreFloat3(&sphereCenterLS, XMVector3TransformCoord(targetPos, V));

	// Ortho frustum in light space encloses scene.
	float l = sphereCenterLS.x - mSceneBounds.Radius;
	float b = sphereCenterLS.y - mSceneBounds.Radius;
	float n = sphereCenterLS.z - mSceneBounds.Radius;
	float r = sphereCenterLS.x + mSceneBounds.Radius;
	float t = sphereCenterLS.y + mSceneBounds.Radius;
	float f = sphereCenterLS.z + mSceneBounds.Radius;
	XMMATRIX P = XMMatrixOrthographicOffCenterLH(l, r, b, t, n, f);

	// Transform NDC space [-1,+1]^2 to texture space [0,1]^2
	XMMATRIX T(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);

	XMMATRIX S = V * P*T;

	XMStoreFloat4x4(&mLightView, V);
	XMStoreFloat4x4(&mLightProj, P);
	XMStoreFloat4x4(&mShadowTransform, S);
}


void MeshViewDemo::BuildScreenQuadGeometryBuffers()
{
	GeometryGenerator::MeshData quad;

	GeometryGenerator geoGen;
	geoGen.CreateFullscreenQuad(quad);

	//
	// Extract the vertex elements we are interested in and pack the
	// vertices of all the meshes into one vertex buffer.
	//

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

	//
	// Pack the indices of all the meshes into one index buffer.
	//

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

















