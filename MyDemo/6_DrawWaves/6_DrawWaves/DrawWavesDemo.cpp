#include "DrawWavesDemo.h"

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	//�����Ǽ���ڴ�й©

	DrawWavesDemo theApp(hInstance);

	if (!theApp.Init())
		return  0;
	return theApp.Run();
}

DrawWavesDemo::DrawWavesDemo(HINSTANCE hinstance):D3DApp(hinstance)
, mLandVB(0), mLandIB(0),mWavesVB(0), mWavesIB(0), mWireframeRS(0), mFX(0), mTech(0),
mfxWorldViewProj(0), mInputLayout(0), mGridIndexCount(0),
mTheta(1.5f*MathHelper::Pi), mPhi(0.1f*MathHelper::Pi), mRadius(200.0f)
{
	m_mainWndCaption = L"D3D11 ��ˮ";
	mLastMousePos.x = 0;
	mLastMousePos.y = 0;
	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&mGridWorld, I);
	XMStoreFloat4x4(&mWavesWorld, I);
	XMStoreFloat4x4(&mView, I);
	XMStoreFloat4x4(&mProj, I);
}


DrawWavesDemo::~DrawWavesDemo()
{
	ReleaseCOM(mLandVB);
	ReleaseCOM(mLandIB);
	ReleaseCOM(mWavesVB);
	ReleaseCOM(mWavesIB);
	ReleaseCOM(mFX);
	ReleaseCOM(mInputLayout);
	ReleaseCOM(mWireframeRS);
}

bool DrawWavesDemo::Init()
{
	if (!D3DApp::Init())
	{
		return false;
	}
	mWaves.Init(200, 200, 0.8f, 0.03f, 3.25f, 0.4f);

	BuildLandGeometryBuffers();
	BuildWavesGeometryBuffers();
	BuildFX();
	BuildVertexLayout();

	D3D11_RASTERIZER_DESC wireframeDesc;
	ZeroMemory(&wireframeDesc, sizeof(D3D11_RASTERIZER_DESC));
	wireframeDesc.FillMode = D3D11_FILL_WIREFRAME;
	wireframeDesc.CullMode = D3D11_CULL_BACK;
	wireframeDesc.FrontCounterClockwise = false;
	wireframeDesc.DepthClipEnable = true;

	HR(m_pD3dDevice->CreateRasterizerState(&wireframeDesc, &mWireframeRS));
	return true;
}

void DrawWavesDemo::OnResize()
{
	D3DApp::OnResize();

	XMMATRIX p = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, D3DApp::AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&mProj, p);

}

void DrawWavesDemo::UpdateScene(float dt)
{
	// �ӽǱ任����
	// ����������ת��Ϊ�ѿ�������
	float x = mRadius * sinf(mPhi)*cosf(mTheta);
	float z = mRadius * sinf(mPhi)*sinf(mTheta);
	float y = mRadius * cosf(mPhi);

	XMVECTOR pos = XMVectorSet(x, y, z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX V = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&mView, V);

	// ÿһ���Ӷ������һ��������ˡ�
	static float t_base = 0.0f;
	if ((m_timer.TotalTime() - t_base) >= 0.25f)
	{
		t_base += 0.25f;

		DWORD i = 5 + rand() % 190;
		DWORD j = 5 + rand() % 190;

		float r = MathHelper::RandF(1.0f, 2.0f);

		mWaves.Disturb(i, j, r);
	}

	mWaves.Update(dt);


	// ����ˮ�Ķ��㻺����

	D3D11_MAPPED_SUBRESOURCE mappedData;
	HR(m_pImmediateContext->Map(mWavesVB, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));

	Vertex* v = reinterpret_cast<Vertex*>(mappedData.pData);
	for (UINT i = 0; i < mWaves.VertexCount(); ++i)
	{
		v[i].Pos = mWaves[i];
		v[i].Color = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	}
	
	m_pImmediateContext->Unmap(mWavesVB, 0);
}

void DrawWavesDemo::DrawScene()
{
	m_pImmediateContext->ClearRenderTargetView(m_pRenderTargetView, reinterpret_cast<const float*>(&Colors::LightSteelBlue));
	m_pImmediateContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// ָ�����벼�֡�ͼԪ�������͡����㻺�塢�������塢��Ⱦ״̬
	m_pImmediateContext->IASetInputLayout(mInputLayout);
	m_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	XMMATRIX view = XMLoadFloat4x4(&mView);
	XMMATRIX proj = XMLoadFloat4x4(&mProj);
	// ��technique��ȡpass�������Ⱦ
	D3DX11_TECHNIQUE_DESC techDesc;
	mTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		//��ɽ��
		m_pImmediateContext->IASetVertexBuffers(0, 1, &mLandVB, &stride, &offset);
		m_pImmediateContext->IASetIndexBuffer(mLandIB, DXGI_FORMAT_R32_UINT, 0);
		XMMATRIX world = XMLoadFloat4x4(&mGridWorld);
		XMMATRIX worldViewProj = world * view*proj;
		mfxWorldViewProj->SetMatrix(reinterpret_cast<float*>(&worldViewProj));
		mTech->GetPassByIndex(p)->Apply(0, m_pImmediateContext);
		m_pImmediateContext->DrawIndexed(mGridIndexCount, 0, 0);

		//��ˮ
		m_pImmediateContext->RSSetState(mWireframeRS);
		m_pImmediateContext->IASetVertexBuffers(0, 1, &mWavesVB, &stride, &offset);
		m_pImmediateContext->IASetIndexBuffer(mWavesIB, DXGI_FORMAT_R32_UINT, 0);

		world = XMLoadFloat4x4(&mWavesWorld);
		worldViewProj = world * view*proj;
		mfxWorldViewProj->SetMatrix(reinterpret_cast<float*>(&worldViewProj));
		mTech->GetPassByIndex(p)->Apply(0, m_pImmediateContext);
		m_pImmediateContext->DrawIndexed(3 * mWaves.TriangleCount(), 0, 0);

		// Restore default.
		m_pImmediateContext->RSSetState(0);
	}
	// ��ʾ
	HR(m_pSwapChain->Present(0, 0));
}

void DrawWavesDemo::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	// һ�����ڲ�������꣬����������붼��Ըô��ڣ����۹���Ƿ��ڴ��ڵı߽���
	SetCapture(m_hMainWnd);
}

void DrawWavesDemo::OnMouseUp(WPARAM btnState, int x, int y)
{
	// ����Ҫ������������Ϣ���ͷŵ�
	ReleaseCapture();
}

void DrawWavesDemo::OnMouseMove(WPARAM btnState, int x, int y)
{
	
	// �����������ӽǽǶ�
	if ((btnState & MK_LBUTTON) != 0)
	{
		// �������Pos��lastPos��x/y�����ϵı仯��dx/dy�ó��Ƕȣ�ÿ�����صľ����൱��1�ȣ��ٽ��Ƕ�תΪ����
		float dx = XMConvertToRadians(0.25f*static_cast<float>(x - mLastMousePos.x));
		float dy = XMConvertToRadians(0.25f*static_cast<float>(y - mLastMousePos.y));

		// ����dx��dy���½Ƕ�
		mTheta += dx;
		mPhi += dy;

		// ���ƽǶ�m_phi
		mPhi = MathHelper::Clamp(mPhi, 0.1f, MathHelper::Pi - 0.1f);
	}
	// �����������ӽǾ���
	else if ((btnState & MK_RBUTTON) != 0)
	{
		// ʹÿ�����ض�Ӧ�ڳ����е�0.005����Ԫ
		float dx = 0.2f*static_cast<float>(x - mLastMousePos.x);
		float dy = 0.2f*static_cast<float>(y - mLastMousePos.y);

		// ��������������������뾶
		mRadius += dx - dy;

		// ����m_radius
		mRadius = MathHelper::Clamp(mRadius, 50.0f, 500.0f);
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}

float DrawWavesDemo::GetHeight(float x, float z) const
{
	return 0.3f*(z*sinf(0.1f*x) + x * cosf(0.1f*z));
}

void DrawWavesDemo::BuildLandGeometryBuffers()
{
	GeometryGenerator::MeshData grid;

	GeometryGenerator geoGen;

	geoGen.CreateGrid(160.0f, 160.0f, 50, 50, grid);

	mGridIndexCount = grid.Indices.size();

	/*��ȡ���Ǹ���Ȥ�Ķ���Ԫ�ز����߶Ⱥ���Ӧ����
	ÿ�����㡣 ���⣬�������ǵĸ߶�Ϊ������ɫ������������
	��ɳ�ĺ�̲���ݵص�ɽ��ѩɽ�塣*/

	std::vector<Vertex> vertices(grid.Vertices.size());
	for (size_t i = 0; i < grid.Vertices.size(); ++i)
	{
		XMFLOAT3 p = grid.Vertices[i].Position;

		p.y = GetHeight(p.x, p.z);

		vertices[i].Pos = p;

		// ��ͬ�ĸ߶����ò�ͬ��ɫ
		if (p.y < -10.0f)
		{
			// ɳ̲ɫ.
			vertices[i].Color = XMFLOAT4(1.0f, 0.96f, 0.62f, 1.0f);
		}
		else if (p.y < 5.0f)
		{
			// ������ɫ��
			vertices[i].Color = XMFLOAT4(0.48f, 0.77f, 0.46f, 1.0f);
		}
		else if (p.y < 12.0f)
		{
			// �����ɫ��
			vertices[i].Color = XMFLOAT4(0.1f, 0.48f, 0.19f, 1.0f);
		}
		else if (p.y < 20.0f)
		{
			// ����ɫ��
			vertices[i].Color = XMFLOAT4(0.45f, 0.39f, 0.34f, 1.0f);
		}
		else
		{
			// ��ѩ��
			vertices[i].Color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		}
	}
	
	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex) * grid.Vertices.size();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	HR(m_pD3dDevice->CreateBuffer(&vbd, &vinitData, &mLandVB));


	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * mGridIndexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &grid.Indices[0];
	HR(m_pD3dDevice->CreateBuffer(&ibd, &iinitData, &mLandIB));
}

void DrawWavesDemo::BuildWavesGeometryBuffers()
{
	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_DYNAMIC;
	vbd.ByteWidth = sizeof(Vertex) * mWaves.VertexCount();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vbd.MiscFlags = 0;
	HR(m_pD3dDevice->CreateBuffer(&vbd, 0, &mWavesVB));


	//���������������� �����������ǹ̶��ģ���������ֻ��Ҫ����������һ�Ρ�
	std::vector<UINT> indices(3 * mWaves.TriangleCount()); // 3������һ����

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

			k += 6; // next quad
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

void DrawWavesDemo::BuildFX()
{
	DWORD shaderFlags = 0;
#if defined( DEBUG ) || defined( _DEBUG )
	shaderFlags |= D3DCOMPILE_DEBUG;
	shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
	ID3D10Blob* compilationMsgs = 0;
	HR(D3DX11CompileEffectFromFile(L"Fx/Color.hlsl", nullptr, nullptr, shaderFlags, 0, m_pD3dDevice, &mFX, &compilationMsgs));
	// compilationMsgs�а�������򾯸���Ϣ
	if (compilationMsgs != 0)
	{
		OutputDebugStringA((char*)compilationMsgs->GetBufferPointer());
		ReleaseCOM(compilationMsgs);
	}

	// ��Effect�л�ȡtechnique����
	mTech = mFX->GetTechniqueByName("ColorTech");
	mfxWorldViewProj = mFX->GetVariableByName("gWorldViewProj")->AsMatrix();

	
}
void DrawWavesDemo::BuildVertexLayout()
{
	//�������벼�� 
	D3D11_INPUT_ELEMENT_DESC ied[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	UINT numLayoutElement = ARRAYSIZE(ied);
	// ��technique�����л�ȡpass��Ϣ
	D3DX11_PASS_DESC passDesc = { 0 };
	mTech->GetPassByIndex(0)->GetDesc(&passDesc);

	HR(m_pD3dDevice->CreateInputLayout(ied, numLayoutElement, passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize, &mInputLayout));
}


