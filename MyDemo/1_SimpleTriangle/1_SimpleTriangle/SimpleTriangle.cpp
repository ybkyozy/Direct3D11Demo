#include "SimpleTriangle.h"

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
	SimpleTriangle theApp(hInstance);
	if (!theApp.Init())
	{
		return 0;
	}
	return theApp.Run();
}


SimpleTriangle::SimpleTriangle(HINSTANCE hinstance) :D3DApp(hinstance)
, mVB(0), mTech(0), mFx(0), mInputLayout(0)
{
	m_mainWndCaption = L"D3D11 简单三角形";
}

SimpleTriangle::~SimpleTriangle()
{
	ReleaseCOM(mVB);
	ReleaseCOM(mFx);
	ReleaseCOM(mInputLayout);
}

bool SimpleTriangle::Init()
{
	if (!D3DApp::Init())
	{
		return false;
	}
	BuidFx();
	BUidBufers();
	return true;
}

void SimpleTriangle::OnResize()
{
	D3DApp::OnResize();
}

void SimpleTriangle::UpdateScene(float dt)
{
	m_pImmediateContext->ClearRenderTargetView(m_pRenderTargetView, Colors::Silver);
	m_pImmediateContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);

	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	m_pImmediateContext->IASetVertexBuffers(0, 1, &mVB, &stride, &offset);
	m_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_pImmediateContext->IASetInputLayout(mInputLayout);

	D3DX11_TECHNIQUE_DESC techDesc;
	mTech->GetDesc(&techDesc);
	for (uint32_t p = 0; p < techDesc.Passes; ++p)
	{
		mTech->GetPassByIndex(p)->Apply(0, m_pImmediateContext);
		m_pImmediateContext->Draw(3, 0);
	}

	m_pSwapChain->Present(0, 0);
}

void SimpleTriangle::DrawScene()
{
	
}

void SimpleTriangle::BuidFx()
{
	uint32_t compitFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
	compitFlags |= D3DCOMPILE_DEBUG;
	compitFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
	ID3DBlob* errorMsg;
	HR(D3DX11CompileEffectFromFile(L"FX/Color.hlsl", NULL, NULL, compitFlags, 0, m_pD3dDevice, &mFx, &errorMsg));
	if (errorMsg)
	{
		OutputDebugStringA((char*)errorMsg->GetBufferPointer());
		ReleaseCOM(errorMsg);
	}
	mTech = mFx->GetTechniqueByName("ColorTech");

}

void SimpleTriangle::BUidBufers()
{
	Vertex vertexs[] =
	{
		XMFLOAT3(-0.5f,-0.5f,0.0f),
		XMFLOAT3(0.0f, 0.5f, 0.0f),
		XMFLOAT3(0.5f, -0.5f, 0.0f)
	};

	D3D11_BUFFER_DESC vbd = { 0 };
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.ByteWidth = sizeof(Vertex) * 3;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;

	D3D11_SUBRESOURCE_DATA vinitData = { 0 };
	vinitData.pSysMem = vertexs;

	HR(m_pD3dDevice->CreateBuffer(&vbd, &vinitData, &mVB));

	//创建输入布局
	D3D11_INPUT_ELEMENT_DESC layoutPos[]=
	{
		{"POSITION",0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	UINT numLayoutElements = ARRAYSIZE(layoutPos);

	D3DX11_PASS_DESC passDesc;
	mTech->GetPassByIndex(0)->GetDesc(&passDesc);

	HR(m_pD3dDevice->CreateInputLayout(layoutPos, numLayoutElements, passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, &mInputLayout));

}
