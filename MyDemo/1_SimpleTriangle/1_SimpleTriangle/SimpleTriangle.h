#pragma once
#include <D3DApp.h>
#include <d3dx11effect.h>
#include <d3dcompiler.h>


struct Vertex
{
	XMFLOAT3 Pos;
};

class SimpleTriangle :
	public D3DApp
{
public:
	SimpleTriangle(HINSTANCE hinstance);
	~SimpleTriangle();

	virtual bool Init() override;
	virtual void OnResize() override;
	virtual void UpdateScene(float dt) override;
	virtual void DrawScene() override;
private:
	ID3D11Buffer * mVB;
	ID3DX11Effect* mFx;
	ID3DX11EffectTechnique* mTech;
	ID3D11InputLayout* mInputLayout;


	void BuidFx();
	void BUidBufers();
};

