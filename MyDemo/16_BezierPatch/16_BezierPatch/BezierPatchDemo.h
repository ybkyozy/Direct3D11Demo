#pragma once
#include <D3DApp.h>
#include <d3dx11effect.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <DDSTextureLoader.h>
#include "MathHelper.h"
#include "GeometryGenerator.h"
#include "LightHelper.h"
#include "Waves.h"
#include "RenderStates.h"
#include "BlurFilter.h"

using namespace DirectX;



class BezierPatchDemo : public D3DApp
{
public:
	BezierPatchDemo(HINSTANCE hinstance);
	virtual ~BezierPatchDemo();

	virtual bool Init() override;
	virtual void OnResize() override;
	virtual void UpdateScene(float dt) override;
	virtual void DrawScene() override;
	virtual void OnMouseDown(WPARAM btnState, int x, int y) override;
	virtual void OnMouseUp(WPARAM btnState, int x, int y) override;
	virtual void OnMouseMove(WPARAM btnState, int x, int y) override;

private:
	void BuildQuadPatchBuffer();
private:
	ID3D11Buffer * mQuadPatchVB;

	XMFLOAT4X4 mView;
	XMFLOAT4X4 mProj;

	XMFLOAT3 mEyePosW;

	float mTheta;
	float mPhi;
	float mRadius;

	POINT mLastMousePos;

};

