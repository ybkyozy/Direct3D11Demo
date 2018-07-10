#include "Effects.h"
#include "Vertex.h"
#include "RenderStates.h"
#include "InstancingAndCullingDemo.h"

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	//上面是检测内存泄漏

	InstancingAndCullingDemo theApp(hInstance);

	if (!theApp.Init())
		return  0;
	return theApp.Run();
}

InstancingAndCullingDemo::InstancingAndCullingDemo(HINSTANCE hinstance):D3DApp(hinstance)
, mSkullVB(0), mSkullIB(0), mSkullIndexCount(0), mInstancedBuffer(0),
mVisibleObjectCount(0), mFrustumCullingEnabled(true)
{
	m_mainWndCaption = L"D3D11 实例 和 剔除";

	srand((unsigned int)time((time_t *)NULL));

	mLastMousePos.x = 0;
	mLastMousePos.y = 0;

	mCam.SetPosition(0.0f, 2.0f, -15.0f);

	XMMATRIX I = XMMatrixIdentity();

	XMMATRIX skullScale = XMMatrixScaling(0.5f, 0.5f, 0.5f);
	XMMATRIX skullOffset = XMMatrixTranslation(0.0f, 1.0f, 0.0f);
	XMStoreFloat4x4(&mSkullWorld, XMMatrixMultiply(skullScale, skullOffset));


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

	mSkullMat.Ambient = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
	mSkullMat.Diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	mSkullMat.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 16.0f);

}


InstancingAndCullingDemo::~InstancingAndCullingDemo()
{
	ReleaseCOM(mSkullVB);
	ReleaseCOM(mSkullIB);
	ReleaseCOM(mInstancedBuffer);

	Effects::DestroyAll();
	InputLayouts::DestroyAll();
	RenderStates::DestroyAll();
}

bool InstancingAndCullingDemo::Init()
{
	if (!D3DApp::Init())
	{
		return false;
	}
	//必须先初始化效果，因为输入布局的初始化依赖它
	Effects::InitAll(m_pD3dDevice);
	InputLayouts::InitAll(m_pD3dDevice);

	BuildSkullGeometryBuffers();
	BuildInstancedBuffer();

	return true;
}

void InstancingAndCullingDemo::OnResize()
{
	D3DApp::OnResize();

	mCam.SetLens(0.25f*MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);

	//在视图空间中构建投影矩阵的平截头体。
	BoundingFrustum::CreateFromMatrix(mCamFrustum, mCam.Proj());
}

void InstancingAndCullingDemo::UpdateScene(float dt)
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

	if (GetAsyncKeyState('1') & 0x8000)
		mFrustumCullingEnabled = true;

	if (GetAsyncKeyState('2') & 0x8000)
		mFrustumCullingEnabled = false;

	//
	// 进行平截头体剔除
	//

	mCam.UpdateViewMatrix();
	mVisibleObjectCount = 0;

	if (mFrustumCullingEnabled)
	{
		XMVECTOR detView = XMMatrixDeterminant(mCam.View());
		XMMATRIX invView = XMMatrixInverse(&detView, mCam.View());

		D3D11_MAPPED_SUBRESOURCE mappedData;
		m_pImmediateContext->Map(mInstancedBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData);

		InstancedData* dataView = reinterpret_cast<InstancedData*>(mappedData.pData);

		for (UINT i = 0; i < mInstancedData.size(); ++i)
		{
			XMMATRIX W = XMLoadFloat4x4(&mInstancedData[i].World);
			XMMATRIX invWorld = XMMatrixInverse(&XMMatrixDeterminant(W), W);

			// 视图空间到对象本地空间
			XMMATRIX toLocal = XMMatrixMultiply(invView, invWorld);

			// 将矩阵分解为各个部分。
			XMVECTOR scale;
			XMVECTOR rotQuat;
			XMVECTOR translation;
			XMMatrixDecompose(&scale, &rotQuat, &translation, toLocal);

			// 将相机视锥体从视图空间转换为对象的本地空间。
			BoundingFrustum localspaceFrustum;
			mCamFrustum.Transform(localspaceFrustum, XMVectorGetX(scale), rotQuat, translation);

			// 在本地空间中执行立方体/锥体相交测试。
			if(localspaceFrustum.Intersects(mSkullBox))
			{
				// 将实例数据写入可见对象的动态顶点缓冲区中。
				dataView[mVisibleObjectCount++] = mInstancedData[i];
			}
		}

		m_pImmediateContext->Unmap(mInstancedBuffer, 0);
	}
	else // No culling enabled, draw all objects.
	{
		D3D11_MAPPED_SUBRESOURCE mappedData;
		m_pImmediateContext->Map(mInstancedBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData);

		InstancedData* dataView = reinterpret_cast<InstancedData*>(mappedData.pData);

		for (UINT i = 0; i < mInstancedData.size(); ++i)
		{
			dataView[mVisibleObjectCount++] = mInstancedData[i];
		}

		m_pImmediateContext->Unmap(mInstancedBuffer, 0);
	}

	std::wostringstream outs;
	outs.precision(6);
	outs << L"D3D11 实例 和 剔除" <<
		L"    " << mVisibleObjectCount <<
		L" 个可见对象 共" << mInstancedData.size();
	m_mainWndCaption = outs.str();

}

void InstancingAndCullingDemo::DrawScene()
{
	m_pImmediateContext->ClearRenderTargetView(m_pRenderTargetView, reinterpret_cast<const float*>(&Colors::Silver));
	m_pImmediateContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	m_pImmediateContext->IASetInputLayout(InputLayouts::InstancedBasic32);
	m_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	UINT stride[2] = { sizeof(Vertex::Basic32), sizeof(InstancedData) };
	UINT offset[2] = { 0,0 };

	ID3D11Buffer* vbs[2] = { mSkullVB, mInstancedBuffer };

	XMMATRIX view = mCam.View();
	XMMATRIX proj = mCam.Proj();
	XMMATRIX viewProj = mCam.ViewProj();

	// Set per frame constants.
	Effects::InstancedBasicFX->SetDirLights(mDirLights);
	Effects::InstancedBasicFX->SetEyePosW(mCam.GetPosition());

	ID3DX11EffectTechnique* activeTech = Effects::InstancedBasicFX->Light3Tech;

	D3DX11_TECHNIQUE_DESC techDesc;
	activeTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		// 画骷髅

		m_pImmediateContext->IASetVertexBuffers(0, 2, vbs, stride, offset);
		m_pImmediateContext->IASetIndexBuffer(mSkullIB, DXGI_FORMAT_R32_UINT, 0);

		XMMATRIX world = XMLoadFloat4x4(&mSkullWorld);
		XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);

		Effects::InstancedBasicFX->SetWorld(world);
		Effects::InstancedBasicFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::InstancedBasicFX->SetViewProj(viewProj);
		Effects::InstancedBasicFX->SetMaterial(mSkullMat);

		activeTech->GetPassByIndex(p)->Apply(0, m_pImmediateContext);
		m_pImmediateContext->DrawIndexedInstanced(mSkullIndexCount, mVisibleObjectCount, 0, 0, 0);
	}



	HR(m_pSwapChain->Present(0, 0));
}

void InstancingAndCullingDemo::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	// 一旦窗口捕获了鼠标，所有鼠标输入都针对该窗口，无论光标是否在窗口的边界内
	SetCapture(m_hMainWnd);
}

void InstancingAndCullingDemo::OnMouseUp(WPARAM btnState, int x, int y)
{
	// 不需要继续获得鼠标消息就释放掉
	ReleaseCapture();
}

void InstancingAndCullingDemo::OnMouseMove(WPARAM btnState, int x, int y)
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
		float dx = 0.1f*static_cast<float>(x - mLastMousePos.x);
		float dy = 0.1f*static_cast<float>(y - mLastMousePos.y);

		mCam.Strafe(dx);
		mCam.fly(-dy);
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}

void InstancingAndCullingDemo::BuildSkullGeometryBuffers()
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

	XMFLOAT3 vMinf3(+MathHelper::Infinity, +MathHelper::Infinity, +MathHelper::Infinity);
	XMFLOAT3 vMaxf3(-MathHelper::Infinity, -MathHelper::Infinity, -MathHelper::Infinity);

	XMVECTOR vMin = XMLoadFloat3(&vMinf3);
	XMVECTOR vMax = XMLoadFloat3(&vMaxf3);
	std::vector<Vertex::Basic32> vertices(vcount);
	for (UINT i = 0; i < vcount; ++i)
	{
		fin >> vertices[i].Pos.x >> vertices[i].Pos.y >> vertices[i].Pos.z;
		fin >> vertices[i].Normal.x >> vertices[i].Normal.y >> vertices[i].Normal.z;

		XMVECTOR P = XMLoadFloat3(&vertices[i].Pos);

		vMin = XMVectorMin(vMin, P);
		vMax = XMVectorMax(vMax, P);
	}

	XMStoreFloat3(&mSkullBox.Center, 0.5f*(vMin + vMax));
	XMStoreFloat3(&mSkullBox.Extents, 0.5f*(vMax - vMin));

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
	// Pack the indices of all the meshes into one index buffer.
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

void InstancingAndCullingDemo::BuildInstancedBuffer()
{
	const int n = 5;
	mInstancedData.resize(n*n*n);

	float width = 200.0f;
	float height = 200.0f;
	float depth = 200.0f;

	float x = -0.5f*width;
	float y = -0.5f*height;
	float z = -0.5f*depth;
	float dx = width / (n - 1);
	float dy = height / (n - 1);
	float dz = depth / (n - 1);
	for (int k = 0; k < n; ++k)
	{
		for (int i = 0; i < n; ++i)
		{
			for (int j = 0; j < n; ++j)
			{
				// 位置沿3D网格实例化。
				mInstancedData[k*n*n + i * n + j].World = XMFLOAT4X4(
					1.0f, 0.0f, 0.0f, 0.0f,
					0.0f, 1.0f, 0.0f, 0.0f,
					0.0f, 0.0f, 1.0f, 0.0f,
					x + j * dx, y + i * dy, z + k * dz, 1.0f);

				// 随机颜色
				mInstancedData[k*n*n + i * n + j].Color.x = MathHelper::RandF(0.0f, 1.0f);
				mInstancedData[k*n*n + i * n + j].Color.y = MathHelper::RandF(0.0f, 1.0f);
				mInstancedData[k*n*n + i * n + j].Color.z = MathHelper::RandF(0.0f, 1.0f);
				mInstancedData[k*n*n + i * n + j].Color.w = 1.0f;
			}
		}
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_DYNAMIC;
	vbd.ByteWidth = sizeof(InstancedData) * mInstancedData.size();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;

	HR(m_pD3dDevice->CreateBuffer(&vbd, 0, &mInstancedBuffer));
}

















