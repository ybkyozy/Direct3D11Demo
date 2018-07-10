#include "Effects.h"
#include "Vertex.h"
#include "RenderStates.h"
#include "PickingDemo.h"

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	//上面是检测内存泄漏

	PickingDemo theApp(hInstance);

	if (!theApp.Init())
		return  0;
	return theApp.Run();
}

PickingDemo::PickingDemo(HINSTANCE hinstance):D3DApp(hinstance)
, mMeshVB(0), mMeshIB(0), mMeshIndexCount(0), mPickedTriangle(-1)
{
	m_mainWndCaption = L"D3D11 拾取";

	mLastMousePos.x = 0;
	mLastMousePos.y = 0;

	mCam.SetPosition(0.0f, 2.0f, -10.0f);

	XMMATRIX MeshScale = XMMatrixScaling(0.5f, 0.5f, 0.5f);
	XMMATRIX MeshOffset = XMMatrixTranslation(0.0f, 1.0f, 0.0f);
	XMStoreFloat4x4(&mMeshWorld, XMMatrixMultiply(MeshScale, MeshOffset));

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

	mMeshMat.Ambient = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
	mMeshMat.Diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	mMeshMat.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 16.0f);

	mPickedTriangleMat.Ambient = XMFLOAT4(0.0f, 0.8f, 0.4f, 1.0f);
	mPickedTriangleMat.Diffuse = XMFLOAT4(0.0f, 0.8f, 0.4f, 1.0f);
	mPickedTriangleMat.Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 16.0f);

}


PickingDemo::~PickingDemo()
{
	ReleaseCOM(mMeshVB);
	ReleaseCOM(mMeshIB);

	Effects::DestroyAll();
	InputLayouts::DestroyAll();
	RenderStates::DestroyAll();
}

bool PickingDemo::Init()
{
	if (!D3DApp::Init())
	{
		return false;
	}
	//必须先初始化效果，因为输入布局的初始化依赖它
	Effects::InitAll(m_pD3dDevice);
	InputLayouts::InitAll(m_pD3dDevice);
	RenderStates::InitAll(m_pD3dDevice);

	BuildMeshGeometryBuffers();

	return true;
}

void PickingDemo::OnResize()
{
	D3DApp::OnResize();

	mCam.SetLens(0.25f*MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);

}

void PickingDemo::UpdateScene(float dt)
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

}

void PickingDemo::DrawScene()
{
	m_pImmediateContext->ClearRenderTargetView(m_pRenderTargetView, reinterpret_cast<const float*>(&Colors::Silver));
	m_pImmediateContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	m_pImmediateContext->IASetInputLayout(InputLayouts::Basic32);
	m_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	UINT stride = sizeof(Vertex::Basic32);
	UINT offset = 0;

	mCam.UpdateViewMatrix();

	XMMATRIX view = mCam.View();
	XMMATRIX proj = mCam.Proj();
	XMMATRIX viewProj = mCam.ViewProj();


	// 设置每帧常量缓冲区
	Effects::BasicFX->SetDirLights(mDirLights);
	Effects::BasicFX->SetEyePosW(mCam.GetPosition());

	ID3DX11EffectTechnique* activeMeshTech = Effects::BasicFX->Light3Tech;

	D3DX11_TECHNIQUE_DESC techDesc;
	activeMeshTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		// 画汽车

		if (GetAsyncKeyState('1') & 0x8000)
			m_pImmediateContext->RSSetState(RenderStates::WireframeRS);

		m_pImmediateContext->IASetVertexBuffers(0, 1, &mMeshVB, &stride, &offset);
		m_pImmediateContext->IASetIndexBuffer(mMeshIB, DXGI_FORMAT_R32_UINT, 0);

		XMMATRIX world = XMLoadFloat4x4(&mMeshWorld);
		XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
		XMMATRIX worldViewProj = world * view*proj;

		Effects::BasicFX->SetWorld(world);
		Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BasicFX->SetWorldViewProj(worldViewProj);
		Effects::BasicFX->SetMaterial(mMeshMat);

		activeMeshTech->GetPassByIndex(p)->Apply(0, m_pImmediateContext);
		m_pImmediateContext->DrawIndexed(mMeshIndexCount, 0, 0);

		// 恢复默认状态
		m_pImmediateContext->RSSetState(0);

		// 再次使用不同的材质绘制选择的三角形以突出显示它。

		if (mPickedTriangle != -1)
		{
			// 将深度测试从 < 更改为 <=，以便如果我们绘制相同的三角形两次，它仍然会通过
			// 深度测试。 这是因为我们用不同的材质重新绘制了挑选的三角形
			// 突出显示它。 

			m_pImmediateContext->OMSetDepthStencilState(RenderStates::LessEqualDSS, 0);

			Effects::BasicFX->SetMaterial(mPickedTriangleMat);
			activeMeshTech->GetPassByIndex(p)->Apply(0, m_pImmediateContext);
			m_pImmediateContext->DrawIndexed(3, 3 * mPickedTriangle, 0);

			// 恢复默认状态
			m_pImmediateContext->OMSetDepthStencilState(0, 0);
		}
	}

	HR(m_pSwapChain->Present(0, 0));
}

void PickingDemo::OnMouseDown(WPARAM btnState, int x, int y)
{
	if ((btnState & MK_LBUTTON) != 0)
	{
		mLastMousePos.x = x;
		mLastMousePos.y = y;

		SetCapture(m_hMainWnd);
	}
	else if ((btnState & MK_RBUTTON) != 0)
	{
		Pick(x, y);
	}
}

void PickingDemo::OnMouseUp(WPARAM btnState, int x, int y)
{
	// 不需要继续获得鼠标消息就释放掉
	ReleaseCapture();
}

void PickingDemo::OnMouseMove(WPARAM btnState, int x, int y)
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
		float dx = 0.01f*static_cast<float>(x - mLastMousePos.x);
		float dy = 0.01f*static_cast<float>(y - mLastMousePos.y);

		mCam.Strafe(dx);
		mCam.fly(-dy);
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}

void PickingDemo::BuildMeshGeometryBuffers()
{
	std::ifstream fin("Models/car.txt");

	if (!fin)
	{
		MessageBox(0, L"Models/car.txt not found.", 0, 0);
		return;
	}

	UINT vcount = 0;
	UINT tcount = 0;
	std::string ignore;

	fin >> ignore >> vcount;
	fin >> ignore >> tcount;
	fin >> ignore >> ignore >> ignore >> ignore;

	XMFLOAT3 vMinf3(+MathHelper::Infinity, +MathHelper::Infinity, +MathHelper::Infinity);
	XMFLOAT3 vMaxf3(-MathHelper::Infinity, -MathHelper::Infinity, -MathHelper::Infinity);

	XMVECTOR vMin = XMLoadFloat3(&vMinf3);
	XMVECTOR vMax = XMLoadFloat3(&vMaxf3);
	mMeshVertices.resize(vcount);
	for (UINT i = 0; i < vcount; ++i)
	{
		fin >> mMeshVertices[i].Pos.x >> mMeshVertices[i].Pos.y >> mMeshVertices[i].Pos.z;
		fin >> mMeshVertices[i].Normal.x >> mMeshVertices[i].Normal.y >> mMeshVertices[i].Normal.z;

		XMVECTOR P = XMLoadFloat3(&mMeshVertices[i].Pos);

		vMin = XMVectorMin(vMin, P);
		vMax = XMVectorMax(vMax, P);
	}

	XMStoreFloat3(&mMeshBox.Center, 0.5f*(vMin + vMax));
	XMStoreFloat3(&mMeshBox.Extents, 0.5f*(vMax - vMin));

	fin >> ignore;
	fin >> ignore;
	fin >> ignore;

	mMeshIndexCount = 3 * tcount;
	mMeshIndices.resize(mMeshIndexCount);
	for (UINT i = 0; i < tcount; ++i)
	{
		fin >> mMeshIndices[i * 3 + 0] >> mMeshIndices[i * 3 + 1] >> mMeshIndices[i * 3 + 2];
	}

	fin.close();

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex::Basic32) * vcount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &mMeshVertices[0];
	HR(m_pD3dDevice->CreateBuffer(&vbd, &vinitData, &mMeshVB));


	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * mMeshIndexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &mMeshIndices[0];
	HR(m_pD3dDevice->CreateBuffer(&ibd, &iinitData, &mMeshIB));
}

void PickingDemo::Pick(int sx, int sy)
{
	XMMATRIX P = mCam.Proj();
	XMFLOAT4X4 fp;
	XMStoreFloat4x4(&fp, P);
	// 在视图空间中计算拾取。
	float vx = (+2.0f*sx / m_clientWidth - 1.0f) / fp._11;
	float vy = (-2.0f*sy / m_clientHeight + 1.0f) / fp._22;

	// 射线在视图空间中的定义
	XMVECTOR rayOrigin = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f); //射线起点
	XMVECTOR rayDir = XMVectorSet(vx, vy, 1.0f, 0.0f); //射线方向

	// 转换射线到本地空间
	XMMATRIX V = mCam.View();
	XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(V), V);

	XMMATRIX W = XMLoadFloat4x4(&mMeshWorld);
	XMMATRIX invWorld = XMMatrixInverse(&XMMatrixDeterminant(W), W);

	XMMATRIX toLocal = XMMatrixMultiply(invView, invWorld);

	rayOrigin = XMVector3TransformCoord(rayOrigin, toLocal);
	rayDir = XMVector3TransformNormal(rayDir, toLocal);

	// 为相交测试制作射线方向单位长度。
	rayDir = XMVector3Normalize(rayDir);

	// 如果我们点击网格的边界框，那么我们可能会选择一个网格三角形，
	// 射线/三角测试也是如此。
	//
	// 如果我们没有碰到边界框，那么我们不可能碰到
	// 网格，所以不要浪费精力进行射线/三角测试。

	// 假设我们还没有选择任何东西，所以初始化为-1。
	mPickedTriangle = -1;
	float tmin = 0.0f;
	
	if (mMeshBox.Intersects(rayOrigin, rayDir, tmin))  //在汽车网格内
	{
		// 找到最近的射线/三角形交叉点。
		tmin = MathHelper::Infinity;
		for (UINT i = 0; i < mMeshIndices.size() / 3; ++i)
		{
			// 三角形索引
			UINT i0 = mMeshIndices[i * 3 + 0];
			UINT i1 = mMeshIndices[i * 3 + 1];
			UINT i2 = mMeshIndices[i * 3 + 2];

			// 三角形顶点
			XMVECTOR v0 = XMLoadFloat3(&mMeshVertices[i0].Pos);
			XMVECTOR v1 = XMLoadFloat3(&mMeshVertices[i1].Pos);
			XMVECTOR v2 = XMLoadFloat3(&mMeshVertices[i2].Pos);

			// 我们必须迭代所有三角形才能找到最近的交点。
			float t = 0.0f;
			if (TriangleTests::Intersects(rayOrigin, rayDir, v0, v1, v2, t)) //射线是否和三角形相交
			{
				if (t < tmin)
				{
					// 这是最近的选择三角形。
					tmin = t;
					mPickedTriangle = i;
				}
			}
		}
	}
}



















