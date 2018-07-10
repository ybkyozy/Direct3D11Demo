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

class DrawShapesDemo : public D3DApp
{
public:
	DrawShapesDemo(HINSTANCE hinstance);
	virtual ~DrawShapesDemo();

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
	ID3D11RasterizerState* mWireframeRS;

	//定义图形的本地坐标空间矩阵
	XMFLOAT4X4 mSphereWorld[10];
	XMFLOAT4X4 mCylWorld[10];
	XMFLOAT4X4 mBoxWorld;
	XMFLOAT4X4 mGridWorld;
	XMFLOAT4X4 mCenterSphere;

	XMFLOAT4X4 mView;
	XMFLOAT4X4 mProj;

	//定义在共享缓冲区中的顶点和索引的偏移和数量
	int mBoxVertexOffset;
	int mGridVertexOffset;
	int mSphereVertexOffset;
	int mCylinderVertexOffset;

	UINT mBoxIndexOffset;
	UINT mGridIndexOffset;
	UINT mSphereIndexOffset;
	UINT mCylinderIndexOffset;

	UINT mBoxIndexCount;
	UINT mGridIndexCount;
	UINT mSphereIndexCount;
	UINT mCylinderIndexCount;

	float mTheta;
	float mPhi;
	float mRadius;

	POINT mLastMousePos;

};

