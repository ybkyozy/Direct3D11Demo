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

class DrawHillsDemo : public D3DApp
{
public:
	DrawHillsDemo(HINSTANCE hinstance);
	virtual ~DrawHillsDemo();

	virtual bool Init() override;
	virtual void OnResize() override;
	virtual void UpdateScene(float dt) override;
	virtual void DrawScene() override;
	virtual void OnMouseDown(WPARAM btnState, int x, int y) override;
	virtual void OnMouseUp(WPARAM btnState, int x, int y) override;
	virtual void OnMouseMove(WPARAM btnState, int x, int y) override;

private:
	float GetHeight(float x, float z)const;
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

	XMFLOAT4X4 mGridWorld;
	XMFLOAT4X4 mView;
	XMFLOAT4X4 mProj;

	UINT mGridIndexCount;

	float mTheta;
	float mPhi;
	float mRadius;

	POINT mLastMousePos;

};

