#include "DynamicCubeMapDemo.h"

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	//�����Ǽ���ڴ�й©

	DynamicCubeMapDemo theApp(hInstance);

	if (!theApp.Init())
		return  0;
	return theApp.Run();
}

DynamicCubeMapDemo::DynamicCubeMapDemo(HINSTANCE hinstance):D3DApp(hinstance)
, mSky(0), mShapesVB(0), mShapesIB(0), mSkullVB(0), mSkullIB(0),
mFloorTexSRV(0), mStoneTexSRV(0), mBrickTexSRV(0),
mDynamicCubeMapDSV(0), mDynamicCubeMapSRV(0),
mSkullIndexCount(0), mLightCount(3)
{
	m_mainWndCaption = L"D3D11 ��̬��������ͼ";

	mLastMousePos.x = 0;
	mLastMousePos.y = 0;

	mCam.SetPosition(0.0f, 2.0f, -15.0f);

	BuildCubeFaceCamera(0.0f, 2.0f, 0.0f);

	for (int i = 0; i < 6; ++i)
	{
		mDynamicCubeMapRTV[i] = 0;
	}

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&mGridWorld, I);

	XMMATRIX boxScale = XMMatrixScaling(3.0f, 1.0f, 3.0f);
	XMMATRIX boxOffset = XMMatrixTranslation(0.0f, 0.5f, 0.0f);
	XMStoreFloat4x4(&mBoxWorld, XMMatrixMultiply(boxScale, boxOffset));

	XMMATRIX centerSphereScale = XMMatrixScaling(2.0f, 2.0f, 2.0f);
	XMMATRIX centerSphereOffset = XMMatrixTranslation(0.0f, 2.0f, 0.0f);
	XMStoreFloat4x4(&mCenterSphereWorld, XMMatrixMultiply(centerSphereScale, centerSphereOffset));

	for (int i = 0; i < 5; ++i)
	{
		XMStoreFloat4x4(&mCylWorld[i * 2 + 0], XMMatrixTranslation(-5.0f, 1.5f, -10.0f + i * 5.0f));
		XMStoreFloat4x4(&mCylWorld[i * 2 + 1], XMMatrixTranslation(+5.0f, 1.5f, -10.0f + i * 5.0f));

		XMStoreFloat4x4(&mSphereWorld[i * 2 + 0], XMMatrixTranslation(-5.0f, 3.5f, -10.0f + i * 5.0f));
		XMStoreFloat4x4(&mSphereWorld[i * 2 + 1], XMMatrixTranslation(+5.0f, 3.5f, -10.0f + i * 5.0f));
	}

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

	mGridMat.Ambient = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	mGridMat.Diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	mGridMat.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 16.0f);
	mGridMat.Reflect = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

	mCylinderMat.Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mCylinderMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mCylinderMat.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 16.0f);
	mCylinderMat.Reflect = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

	mSphereMat.Ambient = XMFLOAT4(0.6f, 0.8f, 1.0f, 1.0f);
	mSphereMat.Diffuse = XMFLOAT4(0.6f, 0.8f, 1.0f, 1.0f);
	mSphereMat.Specular = XMFLOAT4(0.9f, 0.9f, 0.9f, 16.0f);
	mSphereMat.Reflect = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

	mBoxMat.Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mBoxMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mBoxMat.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 16.0f);
	mBoxMat.Reflect = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

	mSkullMat.Ambient = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
	mSkullMat.Diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	mSkullMat.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 16.0f);
	mSkullMat.Reflect = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

	mCenterSphereMat.Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mCenterSphereMat.Diffuse = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mCenterSphereMat.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 16.0f);
	mCenterSphereMat.Reflect = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);

}


DynamicCubeMapDemo::~DynamicCubeMapDemo()
{
	SafeDelete(mSky);

	ReleaseCOM(mShapesVB);
	ReleaseCOM(mShapesIB);
	ReleaseCOM(mSkullVB);
	ReleaseCOM(mSkullIB);
	ReleaseCOM(mFloorTexSRV);
	ReleaseCOM(mStoneTexSRV);
	ReleaseCOM(mBrickTexSRV);
	ReleaseCOM(mDynamicCubeMapDSV);
	ReleaseCOM(mDynamicCubeMapSRV);
	for (int i = 0; i < 6; ++i)
		ReleaseCOM(mDynamicCubeMapRTV[i]);

	Effects::DestroyAll();
	InputLayouts::DestroyAll();

}

bool DynamicCubeMapDemo::Init()
{
	if (!D3DApp::Init())
	{
		return false;
	}
	//�����ȳ�ʼ��Ч������Ϊ���벼�ֵĳ�ʼ��������
	Effects::InitAll(m_pD3dDevice);
	InputLayouts::InitAll(m_pD3dDevice);
	
	mSky = new Sky(m_pD3dDevice, L"Textures/sunsetcube1024.dds", 5000.0f);

	HR(CreateDDSTextureFromFile(m_pD3dDevice, L"Textures/floor.dds", 0, &mFloorTexSRV));

	HR(CreateDDSTextureFromFile(m_pD3dDevice, L"Textures/stone.dds", 0, &mStoneTexSRV));

	HR(CreateDDSTextureFromFile(m_pD3dDevice, L"Textures/bricks.dds", 0, &mBrickTexSRV));

	BuildDynamicCubeMapViews();

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
	// ���������
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
	// ѡ���Դ��
	//
	if (GetAsyncKeyState('0') & 0x8000)
		mLightCount = 0;

	if (GetAsyncKeyState('1') & 0x8000)
		mLightCount = 1;

	if (GetAsyncKeyState('2') & 0x8000)
		mLightCount = 2;

	if (GetAsyncKeyState('3') & 0x8000)
		mLightCount = 3;

	//
	// Χ���������嶯������ͷ��
	//

	XMMATRIX skullScale = XMMatrixScaling(0.2f, 0.2f, 0.2f);
	XMMATRIX skullOffset = XMMatrixTranslation(3.0f, 2.0f, 0.0f);
	XMMATRIX skullLocalRotate = XMMatrixRotationY(2.0f*m_timer.TotalTime());
	XMMATRIX skullGlobalRotate = XMMatrixRotationY(0.5f*m_timer.TotalTime());
	XMStoreFloat4x4(&mSkullWorld, skullScale*skullLocalRotate*skullOffset*skullGlobalRotate);

	mCam.UpdateViewMatrix();
}

void DynamicCubeMapDemo::DrawScene()
{
	ID3D11RenderTargetView* renderTargets[1];

	// ������������ͼ
	m_pImmediateContext->RSSetViewports(1, &mCubeMapViewport);
	for (int i = 0; i < 6; ++i)
	{
		// �����������ͼ�����Ȼ�������
		m_pImmediateContext->ClearRenderTargetView(mDynamicCubeMapRTV[i], reinterpret_cast<const float*>(&Colors::Silver));
		m_pImmediateContext->ClearDepthStencilView(mDynamicCubeMapDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		// ����������ͼ���Ϊ��ȾĿ�ꡣ
		renderTargets[0] = mDynamicCubeMapRTV[i];
		m_pImmediateContext->OMSetRenderTargets(1, renderTargets, mDynamicCubeMapDSV);

		// ������������ĳ������Ƶ�����������ͼ���ϡ�
		DrawScene(mCubeMapCamera[i], false);
	}

	// �ָ�ԭ�����ӿں���ȾĿ�ꡣ
	m_pImmediateContext->RSSetViewports(1, &m_screenViewPort);
	renderTargets[0] = m_pRenderTargetView;
	m_pImmediateContext->OMSetRenderTargets(1, renderTargets, m_pDepthStencilView);

	// Ӳ��������������ͼ�ϵͼ����mipmap
	m_pImmediateContext->GenerateMips(mDynamicCubeMapSRV);

	// ���ڻ��������ĳ�������Ҫ��������
	m_pImmediateContext->ClearRenderTargetView(m_pRenderTargetView, reinterpret_cast<const float*>(&Colors::Silver));
	m_pImmediateContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	DrawScene(mCam, true);

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
	// ����Ҫ������������Ϣ���ͷŵ�
	ReleaseCapture();
}

void DynamicCubeMapDemo::OnMouseMove(WPARAM btnState, int x, int y)
{
	
	// �����������ӽǽǶ�
	if ((btnState & MK_LBUTTON) != 0)
	{
		// �������Pos��lastPos��x/y�����ϵı仯��dx/dy�ó��Ƕȣ�ÿ�����صľ����൱��1�ȣ��ٽ��Ƕ�תΪ����
		float dx = XMConvertToRadians(0.25f*static_cast<float>(x - mLastMousePos.x));
		float dy = XMConvertToRadians(0.25f*static_cast<float>(y - mLastMousePos.y));

		mCam.Pitch(dy);
		mCam.RotateY(dx);
	}
	// �����������ӽǾ���
	else if ((btnState & MK_RBUTTON) != 0)
	{
		// ʹÿ�����ض�Ӧ�ڳ����е�0.005����Ԫ
		float dx = 0.05f*static_cast<float>(x - mLastMousePos.x);
		float dy = 0.05f*static_cast<float>(y - mLastMousePos.y);

		mCam.Strafe(dx);
		mCam.fly(-dy);
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}

void DynamicCubeMapDemo::DrawScene(const Camera & camera, bool drawCenterSphere)
{
	m_pImmediateContext->IASetInputLayout(InputLayouts::Basic32);
	m_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	UINT stride = sizeof(Vertex::Basic32);
	UINT offset = 0;

	XMMATRIX view = camera.View();
	XMMATRIX proj = camera.Proj();
	XMMATRIX viewProj = camera.ViewProj();

	float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };

	// Set per frame constants.
	Effects::BasicFX->SetDirLights(mDirLights);
	Effects::BasicFX->SetEyePosW(mCam.GetPosition());

	// Figure out which technique to use.   

	ID3DX11EffectTechnique* activeTexTech = Effects::BasicFX->Light1TexTech;
	ID3DX11EffectTechnique* activeSkullTech = Effects::BasicFX->Light1Tech;
	ID3DX11EffectTechnique* activeReflectTech = Effects::BasicFX->Light1ReflectTech;
	switch (mLightCount)
	{
	case 1:
		activeTexTech = Effects::BasicFX->Light1TexTech;
		activeSkullTech = Effects::BasicFX->Light1Tech;
		activeReflectTech = Effects::BasicFX->Light1ReflectTech;
		break;
	case 2:
		activeTexTech = Effects::BasicFX->Light2TexTech;
		activeSkullTech = Effects::BasicFX->Light2Tech;
		activeReflectTech = Effects::BasicFX->Light2ReflectTech;
		break;
	case 3:
		activeTexTech = Effects::BasicFX->Light3TexTech;
		activeSkullTech = Effects::BasicFX->Light3Tech;
		activeReflectTech = Effects::BasicFX->Light3ReflectTech;
		break;
	}

	XMMATRIX world;
	XMMATRIX worldInvTranspose;
	XMMATRIX worldViewProj;

	//
	// ������
	//
	D3DX11_TECHNIQUE_DESC techDesc;
	activeSkullTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		m_pImmediateContext->IASetVertexBuffers(0, 1, &mSkullVB, &stride, &offset);
		m_pImmediateContext->IASetIndexBuffer(mSkullIB, DXGI_FORMAT_R32_UINT, 0);

		world = XMLoadFloat4x4(&mSkullWorld);
		worldInvTranspose = MathHelper::InverseTranspose(world);
		worldViewProj = world * view*proj;

		Effects::BasicFX->SetWorld(world);
		Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BasicFX->SetWorldViewProj(worldViewProj);
		Effects::BasicFX->SetTexTransform(XMMatrixIdentity());
		Effects::BasicFX->SetMaterial(mSkullMat);

		activeSkullTech->GetPassByIndex(p)->Apply(0, m_pImmediateContext);
		m_pImmediateContext->DrawIndexed(mSkullIndexCount, 0, 0);
	}

	m_pImmediateContext->IASetVertexBuffers(0, 1, &mShapesVB, &stride, &offset);
	m_pImmediateContext->IASetIndexBuffer(mShapesIB, DXGI_FORMAT_R32_UINT, 0);

	//
	// ����û���κ������巴�������Բ���壬����������塣
	// 

	activeTexTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		// ������
		world = XMLoadFloat4x4(&mGridWorld);
		worldInvTranspose = MathHelper::InverseTranspose(world);
		worldViewProj = world * view*proj;

		Effects::BasicFX->SetWorld(world);
		Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BasicFX->SetWorldViewProj(worldViewProj);
		Effects::BasicFX->SetTexTransform(XMMatrixScaling(6.0f, 8.0f, 1.0f));
		Effects::BasicFX->SetMaterial(mGridMat);
		Effects::BasicFX->SetDiffuseMap(mFloorTexSRV);

		activeTexTech->GetPassByIndex(p)->Apply(0, m_pImmediateContext);
		m_pImmediateContext->DrawIndexed(mGridIndexCount, mGridIndexOffset, mGridVertexOffset);

		// ��������
		world = XMLoadFloat4x4(&mBoxWorld);
		worldInvTranspose = MathHelper::InverseTranspose(world);
		worldViewProj = world * view*proj;

		Effects::BasicFX->SetWorld(world);
		Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BasicFX->SetWorldViewProj(worldViewProj);
		Effects::BasicFX->SetTexTransform(XMMatrixIdentity());
		Effects::BasicFX->SetMaterial(mBoxMat);
		Effects::BasicFX->SetDiffuseMap(mStoneTexSRV);

		activeTexTech->GetPassByIndex(p)->Apply(0, m_pImmediateContext);
		m_pImmediateContext->DrawIndexed(mBoxIndexCount, mBoxIndexOffset, mBoxVertexOffset);

		// ��Բ��
		for (int i = 0; i < 10; ++i)
		{
			world = XMLoadFloat4x4(&mCylWorld[i]);
			worldInvTranspose = MathHelper::InverseTranspose(world);
			worldViewProj = world * view*proj;

			Effects::BasicFX->SetWorld(world);
			Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
			Effects::BasicFX->SetWorldViewProj(worldViewProj);
			Effects::BasicFX->SetTexTransform(XMMatrixIdentity());
			Effects::BasicFX->SetMaterial(mCylinderMat);
			Effects::BasicFX->SetDiffuseMap(mBrickTexSRV);

			activeTexTech->GetPassByIndex(p)->Apply(0, m_pImmediateContext);
			m_pImmediateContext->DrawIndexed(mCylinderIndexCount, mCylinderIndexOffset, mCylinderVertexOffset);
		}

		// ������
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
			Effects::BasicFX->SetDiffuseMap(mStoneTexSRV);

			activeTexTech->GetPassByIndex(p)->Apply(0, m_pImmediateContext);
			m_pImmediateContext->DrawIndexed(mSphereIndexCount, mSphereIndexOffset, mSphereVertexOffset);
		}
	}

	//
	// �ö�̬��������ͼ�����������塣
	//
	if (drawCenterSphere)
	{
		activeReflectTech->GetDesc(&techDesc);
		for (UINT p = 0; p < techDesc.Passes; ++p)
		{
			//����������

			world = XMLoadFloat4x4(&mCenterSphereWorld);
			worldInvTranspose = MathHelper::InverseTranspose(world);
			worldViewProj = world * view*proj;

			Effects::BasicFX->SetWorld(world);
			Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
			Effects::BasicFX->SetWorldViewProj(worldViewProj);
			Effects::BasicFX->SetTexTransform(XMMatrixIdentity());
			Effects::BasicFX->SetMaterial(mCenterSphereMat);
			Effects::BasicFX->SetDiffuseMap(mStoneTexSRV);
			Effects::BasicFX->SetCubeMap(mDynamicCubeMapSRV);

			activeReflectTech->GetPassByIndex(p)->Apply(0, m_pImmediateContext);
			m_pImmediateContext->DrawIndexed(mSphereIndexCount, mSphereIndexOffset, mSphereVertexOffset);
		}
	}

	mSky->Draw(m_pImmediateContext, camera);

	// �ָ�Ĭ��״̬����ΪSkyFX��Ч���ļ��л�������ǡ�
	m_pImmediateContext->RSSetState(0);
	m_pImmediateContext->OMSetDepthStencilState(0, 0);
}

void DynamicCubeMapDemo::BuildCubeFaceCamera(float x, float y, float z)
{
	// ���ɸ���λ�õ���������ͼ��
	XMFLOAT3 center(x, y, z);
	XMFLOAT3 worldUp(0.0f, 1.0f, 0.0f);

	// ��ÿ�������ῴ��
	XMFLOAT3 targets[6] =
	{
		XMFLOAT3(x + 1.0f, y, z), // +X
		XMFLOAT3(x - 1.0f, y, z), // -X
		XMFLOAT3(x, y + 1.0f, z), // +Y
		XMFLOAT3(x, y - 1.0f, z), // -Y
		XMFLOAT3(x, y, z + 1.0f), // +Z
		XMFLOAT3(x, y, z - 1.0f)  // -Z
	};

	// ʹ�������������ϣ�0,1,0������+ Y / -Y֮������з��� ����Щ����£�����
	// �������¿�+ Y��-Y������������Ҫһ����ͬ�ġ����ϡ�������
	XMFLOAT3 ups[6] =
	{
		XMFLOAT3(0.0f, 1.0f, 0.0f),  // +X
		XMFLOAT3(0.0f, 1.0f, 0.0f),  // -X
		XMFLOAT3(0.0f, 0.0f, -1.0f), // +Y
		XMFLOAT3(0.0f, 0.0f, +1.0f), // -Y
		XMFLOAT3(0.0f, 1.0f, 0.0f),	 // +Z
		XMFLOAT3(0.0f, 1.0f, 0.0f)	 // -Z
	};

	for (int i = 0; i < 6; ++i)
	{
		mCubeMapCamera[i].LookAt(center, targets[i], ups[i]);
		mCubeMapCamera[i].SetLens(0.5f*XM_PI, 1.0f, 0.1f, 1000.0f);
		mCubeMapCamera[i].UpdateViewMatrix();
	}
}

void DynamicCubeMapDemo::BuildDynamicCubeMapViews()
{
	//
	// ��������ͼ��һ������6��Ԫ�ص������������С�
	//

	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = CubeMapSize;
	texDesc.Height = CubeMapSize;
	texDesc.MipLevels = 0;
	texDesc.ArraySize = 6;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS | D3D11_RESOURCE_MISC_TEXTURECUBE;

	ID3D11Texture2D* cubeTex = 0;
	HR(m_pD3dDevice->CreateTexture2D(&texDesc, 0, &cubeTex));

	//
	// Ϊÿ����������ͼ�洴��һ����ȾĿ����ͼ
	//�������������е�ÿ��Ԫ�أ���
	// 

	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
	rtvDesc.Format = texDesc.Format;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
	rtvDesc.Texture2DArray.ArraySize = 1;
	rtvDesc.Texture2DArray.MipSlice = 0;

	for (int i = 0; i < 6; ++i)
	{
		rtvDesc.Texture2DArray.FirstArraySlice = i;
		HR(m_pD3dDevice->CreateRenderTargetView(cubeTex, &rtvDesc, &mDynamicCubeMapRTV[i]));
	}

	//
	// Ϊ��������ͼ������ɫ����Դ��ͼ��
	//

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = texDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.TextureCube.MostDetailedMip = 0;
	srvDesc.TextureCube.MipLevels = -1;

	HR(m_pD3dDevice->CreateShaderResourceView(cubeTex, &srvDesc, &mDynamicCubeMapSRV));

	ReleaseCOM(cubeTex);

	//
	// ������Ҫ�������������Ⱦ����������ͼ��
	// ����������ͼ��ͼ��ͬ�ķֱ��ʡ�
	//

	D3D11_TEXTURE2D_DESC depthTexDesc;
	depthTexDesc.Width = CubeMapSize;
	depthTexDesc.Height = CubeMapSize;
	depthTexDesc.MipLevels = 1;
	depthTexDesc.ArraySize = 1;
	depthTexDesc.SampleDesc.Count = 1;
	depthTexDesc.SampleDesc.Quality = 0;
	depthTexDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthTexDesc.Usage = D3D11_USAGE_DEFAULT;
	depthTexDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthTexDesc.CPUAccessFlags = 0;
	depthTexDesc.MiscFlags = 0;

	ID3D11Texture2D* depthTex = 0;
	HR(m_pD3dDevice->CreateTexture2D(&depthTexDesc, 0, &depthTex));

	// Ϊ���������崴�����ģ����ͼ
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Format = depthTexDesc.Format;
	dsvDesc.Flags = 0;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;
	HR(m_pD3dDevice->CreateDepthStencilView(depthTex, &dsvDesc, &mDynamicCubeMapDSV));

	ReleaseCOM(depthTex);

	//
	// ���ڻ��Ƶ���������ͼ���ӿڡ�
	// 

	mCubeMapViewport.TopLeftX = 0.0f;
	mCubeMapViewport.TopLeftY = 0.0f;
	mCubeMapViewport.Width = (float)CubeMapSize;
	mCubeMapViewport.Height = (float)CubeMapSize;
	mCubeMapViewport.MinDepth = 0.0f;
	mCubeMapViewport.MaxDepth = 1.0f;
}

void DynamicCubeMapDemo::BuildShapeGeometryBuffers()
{
	GeometryGenerator::MeshData box;
	GeometryGenerator::MeshData grid;
	GeometryGenerator::MeshData sphere;
	GeometryGenerator::MeshData cylinder;

	GeometryGenerator geoGen;
	geoGen.CreateBox(1.0f, 1.0f, 1.0f, box);
	geoGen.CreateGrid(20.0f, 30.0f, 60, 40, grid);
	geoGen.CreateSphere(0.5f, 20, 20, sphere);
	geoGen.CreateCylinder(0.5f, 0.3f, 3.0f, 20, 20, cylinder);

	// �������Ӷ��㻺������ÿ������Ķ���ƫ������
	mBoxVertexOffset = 0;
	mGridVertexOffset = box.Vertices.size();
	mSphereVertexOffset = mGridVertexOffset + grid.Vertices.size();
	mCylinderVertexOffset = mSphereVertexOffset + sphere.Vertices.size();

	// ����ÿ�����������������
	mBoxIndexCount = box.Indices.size();
	mGridIndexCount = grid.Indices.size();
	mSphereIndexCount = sphere.Indices.size();
	mCylinderIndexCount = cylinder.Indices.size();

	// ��������������������ÿ���������ʼ������
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
	// ��ȡ���Ǹ���Ȥ�Ķ���Ԫ�ز����
	// ��������Ķ������һ�����㻺������
	//

	std::vector<Vertex::Basic32> vertices(totalVertexCount);

	UINT k = 0;
	for (size_t i = 0; i < box.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = box.Vertices[i].Position;
		vertices[k].Normal = box.Vertices[i].Normal;
		vertices[k].Tex = box.Vertices[i].TexC;
	}

	for (size_t i = 0; i < grid.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = grid.Vertices[i].Position;
		vertices[k].Normal = grid.Vertices[i].Normal;
		vertices[k].Tex = grid.Vertices[i].TexC;
	}

	for (size_t i = 0; i < sphere.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = sphere.Vertices[i].Position;
		vertices[k].Normal = sphere.Vertices[i].Normal;
		vertices[k].Tex = sphere.Vertices[i].TexC;
	}

	for (size_t i = 0; i < cylinder.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = cylinder.Vertices[i].Position;
		vertices[k].Normal = cylinder.Vertices[i].Normal;
		vertices[k].Tex = cylinder.Vertices[i].TexC;
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex::Basic32) * totalVertexCount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	HR(m_pD3dDevice->CreateBuffer(&vbd, &vinitData, &mShapesVB));

	//
	// ��������������������һ�������������С�
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
	// ��������������������һ�������������С�
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





















