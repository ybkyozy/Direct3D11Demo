#pragma once
#include <D3DApp.h>
#include <d3dx11effect.h>
#include <d3dcompiler.h>
#include <SimpleMath.h>
#include "MathHelper.h"
#include "GeometryGenerator.h"

using namespace DirectX::SimpleMath;

struct Vertex
{
	XMFLOAT3 Pos;
	XMFLOAT4 Color;
};

class DrawSkullDemo : public D3DApp
{
public:
	DrawSkullDemo(HINSTANCE hinstance);
	virtual ~DrawSkullDemo();

	virtual bool Init() override;
	virtual void OnResize() override;
	virtual void UpdateScene(float dt) override;
	virtual void DrawScene() override;
	virtual void OnMouseDown(WPARAM btnState, int x, int y) override;
	virtual void OnMouseUp(WPARAM btnState, int x, int y) override;
	virtual void OnMouseMove(WPARAM btnState, int x, int y) override;

private:
	void BuildGeometryBuffers();
	void BuildFX();
	void BuildVertexLayout();
private:
	ID3D11Buffer * mVB;
	ID3D11Buffer* mIB;

	ID3DX11Effect* mFX;
	ID3DX11EffectTechnique* mTech;
	ID3DX11EffectMatrixVariable* mfxWorldViewProj;

	ID3D11InputLayout* mInputLayout;
	ID3D11RasterizerState* mWireframeRS;

	XMFLOAT4X4 mSkullWorld;

	XMFLOAT4X4 mView;
	XMFLOAT4X4 mProj;

	UINT mSkullIndexCount;

	float mTheta;
	float mPhi;
	float mRadius;

	POINT mLastMousePos;

};

