#include "DrawSkullDemo.h"

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	//�����Ǽ���ڴ�й©

	DrawSkullDemo theApp(hInstance);

	if (!theApp.Init())
		return  0;
	return theApp.Run();
}

DrawSkullDemo::DrawSkullDemo(HINSTANCE hinstance):D3DApp(hinstance)
, mVB(0), mIB(0), mFX(0), mTech(0),
mfxWorldViewProj(0), mInputLayout(0), mSkullIndexCount(0), mWireframeRS(0),
mTheta(1.5f*MathHelper::Pi), mPhi(0.1f*MathHelper::Pi), mRadius(20.0f)
{
	m_mainWndCaption = L"D3D11 ������";
	mLastMousePos.x = 0;
	mLastMousePos.y = 0;
	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&mView, I);
	XMStoreFloat4x4(&mProj, I);

	XMMATRIX T = XMMatrixTranslation(0.0f, -2.0f, 0.0f);
	XMStoreFloat4x4(&mSkullWorld, T);
}


DrawSkullDemo::~DrawSkullDemo()
{
	ReleaseCOM(mVB);
	ReleaseCOM(mIB);
	ReleaseCOM(mFX);
	ReleaseCOM(mInputLayout);
	ReleaseCOM(mWireframeRS);
}

bool DrawSkullDemo::Init()
{
	if (!D3DApp::Init())
	{
		return false;
	}

	BuildGeometryBuffers();
	BuildFX();
	BuildVertexLayout();

	//������դ��״̬Ϊ�߿�ģʽ
	D3D11_RASTERIZER_DESC wireframeDesc;
	ZeroMemory(&wireframeDesc, sizeof(D3D11_RASTERIZER_DESC));
	wireframeDesc.FillMode = D3D11_FILL_WIREFRAME;
	wireframeDesc.CullMode = D3D11_CULL_BACK;
	wireframeDesc.FrontCounterClockwise = false;
	wireframeDesc.DepthClipEnable = true;
	HR(m_pD3dDevice->CreateRasterizerState(&wireframeDesc, &mWireframeRS));


	return true;
}

void DrawSkullDemo::OnResize()
{
	D3DApp::OnResize();

	XMMATRIX p = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, D3DApp::AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&mProj, p);

}

void DrawSkullDemo::UpdateScene(float dt)
{
	// �ӽǱ任����
	// ����������ת��Ϊ�ѿ�������
	float x = mRadius * sinf(mPhi)*cosf(mTheta);
	float z = mRadius * sinf(mPhi)*sinf(mTheta);
	float y = mRadius * cosf(mPhi);

	XMVECTOR pos = XMVectorSet(x, y, z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	//��ȡ�۲�����
	XMMATRIX V = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&mView, V);
}

void DrawSkullDemo::DrawScene()
{
	m_pImmediateContext->ClearRenderTargetView(m_pRenderTargetView, reinterpret_cast<const float*>(&Colors::LightSteelBlue));
	m_pImmediateContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// ָ�����벼�֡�ͼԪ�������͡����㻺�塢�������塢��Ⱦ״̬
	m_pImmediateContext->IASetInputLayout(mInputLayout);
	m_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	m_pImmediateContext->IASetVertexBuffers(0, 1, &mVB, &stride, &offset);
	m_pImmediateContext->IASetIndexBuffer(mIB, DXGI_FORMAT_R32_UINT, 0);

	// ���ù�դ��״̬
	m_pImmediateContext->RSSetState(mWireframeRS);

	XMMATRIX view = XMLoadFloat4x4(&mView);
	XMMATRIX proj = XMLoadFloat4x4(&mProj);
	XMMATRIX world = XMLoadFloat4x4(&mSkullWorld);
	XMMATRIX worldViewProj = world * view * proj;

	mfxWorldViewProj->SetMatrix(reinterpret_cast<float*>(&worldViewProj));

	// ��technique��ȡpass�������Ⱦ
	D3DX11_TECHNIQUE_DESC techDesc;
	mTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		// ������.
		mTech->GetPassByIndex(p)->Apply(0, m_pImmediateContext);
		m_pImmediateContext->DrawIndexed(mSkullIndexCount, 0, 0);

	}
	// ��ʾ
	HR(m_pSwapChain->Present(0, 0));
}

void DrawSkullDemo::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	// һ�����ڲ�������꣬����������붼��Ըô��ڣ����۹���Ƿ��ڴ��ڵı߽���
	SetCapture(m_hMainWnd);
}

void DrawSkullDemo::OnMouseUp(WPARAM btnState, int x, int y)
{
	// ����Ҫ������������Ϣ���ͷŵ�
	ReleaseCapture();
}

void DrawSkullDemo::OnMouseMove(WPARAM btnState, int x, int y)
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
		mRadius = MathHelper::Clamp(mRadius, 5.0f, 50.0f);
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}


void DrawSkullDemo::BuildGeometryBuffers()
{
	std::ifstream fin("Models/skull.txt");
	if (!fin)
	{
		MessageBox(0, L"Models/skull.txt �ļ�δ�ҵ�", 0, 0);
		return;
	}
	UINT vcount = 0;
	UINT tcount = 0;
	std::string ignore;

	fin >> ignore >> vcount;
	fin >> ignore >> tcount;
	fin >> ignore >> ignore >> ignore >> ignore;

	float nx, ny, nz;
	XMFLOAT4 black(0.0f, 0.0f, 0.0f, 1.0f);

	std::vector<Vertex> vertices(vcount);
	for (UINT i = 0; i < vcount; ++i)
	{
		fin >> vertices[i].Pos.x >> vertices[i].Pos.y >> vertices[i].Pos.z;

		vertices[i].Color = black;

		// ���Demoû��ʹ�÷���
		fin >> nx >> ny >> nz;
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
	vbd.ByteWidth = sizeof(Vertex) * vcount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	HR(m_pD3dDevice->CreateBuffer(&vbd, &vinitData, &mVB));


	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * mSkullIndexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &indices[0];
	HR(m_pD3dDevice->CreateBuffer(&ibd, &iinitData, &mIB));
}

void DrawSkullDemo::BuildFX()
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
void DrawSkullDemo::BuildVertexLayout()
{
	//�������벼�� 
	D3D11_INPUT_ELEMENT_DESC ied[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	UINT numLayoutElement = ARRAYSIZE(ied);
	// ��technique�����л�ȡpass��Ϣ
	D3DX11_PASS_DESC passDesc;
	mTech->GetPassByIndex(0)->GetDesc(&passDesc);

	HR(m_pD3dDevice->CreateInputLayout(ied, numLayoutElement, passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize, &mInputLayout));
}


