#pragma once
#include <D3DApp.h>
#include <d3dx11effect.h>
#include <d3dcompiler.h>
#include <SimpleMath.h>
#include "MathHelper.h"
#include "GeometryGenerator.h"
#include "Waves.h"

using namespace DirectX::SimpleMath;

struct Vertex
{
	XMFLOAT3 Pos;
	XMFLOAT4 Color;
};

class DrawWavesDemo : public D3DApp
{
public:
	DrawWavesDemo(HINSTANCE hinstance);
	virtual ~DrawWavesDemo();

	virtual bool Init() override;
	virtual void OnResize() override;
	virtual void UpdateScene(float dt) override;
	virtual void DrawScene() override;
	virtual void OnMouseDown(WPARAM btnState, int x, int y) override;
	virtual void OnMouseUp(WPARAM btnState, int x, int y) override;
	virtual void OnMouseMove(WPARAM btnState, int x, int y) override;

private:
	float GetHeight(float x, float z)const;
	void BuildLandGeometryBuffers();
	void BuildWavesGeometryBuffers();
	void BuildFX();
	void BuildVertexLayout();
private:
	ID3D11Buffer* mLandVB;
	ID3D11Buffer* mLandIB;
	ID3D11Buffer* mWavesVB;
	ID3D11Buffer* mWavesIB;

	ID3DX11Effect* mFX;
	ID3DX11EffectTechnique* mTech;
	ID3DX11EffectMatrixVariable* mfxWorldViewProj;

	ID3D11InputLayout* mInputLayout;
	ID3D11RasterizerState* mWireframeRS;


	XMFLOAT4X4 mGridWorld;
	XMFLOAT4X4 mWavesWorld;

	XMFLOAT4X4 mView;
	XMFLOAT4X4 mProj;

	UINT mGridIndexCount;
	Waves mWaves;

	float mTheta;
	float mPhi;
	float mRadius;

	POINT mLastMousePos;

};

