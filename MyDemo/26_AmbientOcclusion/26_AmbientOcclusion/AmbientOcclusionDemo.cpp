#include "AmbientOcclusionDemo.h"

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	//上面是检测内存泄漏

	AmbientOcclusionDemo theApp(hInstance);

	if (!theApp.Init())
		return  0;
	return theApp.Run();
}

AmbientOcclusionDemo::AmbientOcclusionDemo(HINSTANCE hinstance):D3DApp(hinstance)
, mSkullVB(0), mSkullIB(0), mSkullIndexCount(0)
{
	m_mainWndCaption = L"D3D11 Ambient Occlusion(环境遮蔽)";

	mLastMousePos.x = 0;
	mLastMousePos.y = 0;

	mCam.SetPosition(0.0f, 5.0f, -5.0f);
	mCam.LookAt(
		XMFLOAT3(-4.0f, 4.0f, -4.0f),
		XMFLOAT3(0.0f, 2.2f, 0.0f),
		XMFLOAT3(0.0f, 1.0f, 0.0f));

	XMMATRIX skullScale = XMMatrixScaling(0.5f, 0.5f, 0.5f);
	XMMATRIX skullOffset = XMMatrixTranslation(0.0f, 1.0f, 0.0f);
	XMStoreFloat4x4(&mSkullWorld, XMMatrixMultiply(skullScale, skullOffset));

}


AmbientOcclusionDemo::~AmbientOcclusionDemo()
{
	ReleaseCOM(mSkullVB);
	ReleaseCOM(mSkullIB);

	Effects::DestroyAll();
	InputLayouts::DestroyAll();
}

bool AmbientOcclusionDemo::Init()
{
	if (!D3DApp::Init())
	{
		return false;
	}
	//必须先初始化效果，因为输入布局的初始化依赖它
	Effects::InitAll(m_pD3dDevice);
	InputLayouts::InitAll(m_pD3dDevice);

	
	BuildSkullGeometryBuffers();

	return true;
}

void AmbientOcclusionDemo::OnResize()
{
	D3DApp::OnResize();

	mCam.SetLens(0.25f*MathHelper::Pi, AspectRatio(), 1.0f, 3000.0f);

}

void AmbientOcclusionDemo::UpdateScene(float dt)
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


	mCam.UpdateViewMatrix();
}

void AmbientOcclusionDemo::DrawScene()
{
	m_pImmediateContext->ClearRenderTargetView(m_pRenderTargetView, reinterpret_cast<const float*>(&Colors::Silver));
	m_pImmediateContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	m_pImmediateContext->IASetInputLayout(InputLayouts::AmbientOcclusion);
	m_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	UINT stride = sizeof(Vertex::AmbientOcclusion);
	UINT offset = 0;

	mCam.UpdateViewMatrix();

	XMMATRIX view = mCam.View();
	XMMATRIX proj = mCam.Proj();
	XMMATRIX viewProj = mCam.ViewProj();


	ID3DX11EffectTechnique* activeSkullTech = Effects::AmbientOcclusionFX->AmbientOcclusionTech;

	D3DX11_TECHNIQUE_DESC techDesc;
	activeSkullTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		// Draw the skull.

		m_pImmediateContext->IASetVertexBuffers(0, 1, &mSkullVB, &stride, &offset);
		m_pImmediateContext->IASetIndexBuffer(mSkullIB, DXGI_FORMAT_R32_UINT, 0);

		XMMATRIX world = XMLoadFloat4x4(&mSkullWorld);
		XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
		XMMATRIX worldViewProj = world * view*proj;

		Effects::AmbientOcclusionFX->SetWorldViewProj(worldViewProj);

		activeSkullTech->GetPassByIndex(p)->Apply(0, m_pImmediateContext);
		m_pImmediateContext->DrawIndexed(mSkullIndexCount, 0, 0);
	}
	HR(m_pSwapChain->Present(0, 0));

}

void AmbientOcclusionDemo::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	SetCapture(m_hMainWnd);
}

void AmbientOcclusionDemo::OnMouseUp(WPARAM btnState, int x, int y)
{
	// 不需要继续获得鼠标消息就释放掉
	ReleaseCapture();
}

void AmbientOcclusionDemo::OnMouseMove(WPARAM btnState, int x, int y)
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


void AmbientOcclusionDemo::BuildVertexAmbientOcclusion(std::vector<Vertex::AmbientOcclusion>& vertices, const std::vector<UINT>& indices)
{
	UINT vcount = vertices.size();
	UINT tcount = indices.size() / 3;

	std::vector<XMFLOAT3> positions(vcount);
	for (UINT i = 0; i < vcount; ++i)
		positions[i] = vertices[i].Pos;

	Octree octree;
	octree.Build(positions, indices);

	// For each vertex, count how many triangles contain the vertex.
	std::vector<int> vertexSharedCount(vcount);
	for (UINT i = 0; i < tcount; ++i)
	{
		UINT i0 = indices[i * 3 + 0];
		UINT i1 = indices[i * 3 + 1];
		UINT i2 = indices[i * 3 + 2];

		XMVECTOR v0 = XMLoadFloat3(&vertices[i0].Pos);
		XMVECTOR v1 = XMLoadFloat3(&vertices[i1].Pos);
		XMVECTOR v2 = XMLoadFloat3(&vertices[i2].Pos);

		XMVECTOR edge0 = v1 - v0;
		XMVECTOR edge1 = v2 - v0;

		XMVECTOR normal = XMVector3Normalize(XMVector3Cross(edge0, edge1));

		XMVECTOR centroid = (v0 + v1 + v2) / 3.0f;

		// Offset to avoid self intersection.
		centroid += 0.001f*normal;

		const int NumSampleRays = 32;
		float numUnoccluded = 0;
		for (int j = 0; j < NumSampleRays; ++j)
		{
			XMVECTOR randomDir = MathHelper::RandHemisphereUnitVec3(normal);

			// TODO: Technically we should not count intersections that are far 
			// away as occluding the triangle, but this is OK for demo.
			if (!octree.RayOctreeIntersect(centroid, randomDir))
			{
				numUnoccluded++;
			}
		}

		float ambientAccess = numUnoccluded / NumSampleRays;

		// Average with vertices that share this face.
		vertices[i0].AmbientAccess += ambientAccess;
		vertices[i1].AmbientAccess += ambientAccess;
		vertices[i2].AmbientAccess += ambientAccess;

		vertexSharedCount[i0]++;
		vertexSharedCount[i1]++;
		vertexSharedCount[i2]++;
	}

	// Finish average by dividing by the number of samples we added.
	for (UINT i = 0; i < vcount; ++i)
	{
		vertices[i].AmbientAccess /= vertexSharedCount[i];
	}
}

void AmbientOcclusionDemo::BuildSkullGeometryBuffers()
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

	std::vector<Vertex::AmbientOcclusion> vertices(vcount);
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


	BuildVertexAmbientOcclusion(vertices, indices);


	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex::AmbientOcclusion) * vcount;
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



















