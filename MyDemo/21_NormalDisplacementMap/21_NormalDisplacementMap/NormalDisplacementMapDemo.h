#pragma once
#include <D3DApp.h>
#include <d3dx11effect.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <DDSTextureLoader.h>
#include <DirectXCollision.h>
#include "MathHelper.h"
#include "GeometryGenerator.h"
#include "LightHelper.h"
#include "Camera.h"
#include "Effects.h"
#include "Vertex.h"
#include "Sky.h"
#include "RenderStates.h"

using namespace DirectX;

enum RenderOptions
{
	RenderOptionsBasic = 0,
	RenderOptionsNormalMap = 1,
	RenderOptionsDisplacementMap = 2
};

class DynamicCubeMapDemo : public D3DApp
{
public:
	DynamicCubeMapDemo(HINSTANCE hinstance);
	virtual ~DynamicCubeMapDemo();

	virtual bool Init() override;
	virtual void OnResize() override;
	virtual void UpdateScene(float dt) override;
	virtual void DrawScene() override;
	virtual void OnMouseDown(WPARAM btnState, int x, int y) override;
	virtual void OnMouseUp(WPARAM btnState, int x, int y) override;
	virtual void OnMouseMove(WPARAM btnState, int x, int y) override;

private:
	void BuildShapeGeometryBuffers();
	void BuildSkullGeometryBuffers();
private:
	Sky * mSky;

	ID3D11Buffer* mShapesVB;
	ID3D11Buffer* mShapesIB;

	ID3D11Buffer* mSkullVB;
	ID3D11Buffer* mSkullIB;

	ID3D11Buffer* mSkySphereVB;
	ID3D11Buffer* mSkySphereIB;

	ID3D11ShaderResourceView* mStoneTexSRV;
	ID3D11ShaderResourceView* mBrickTexSRV;

	ID3D11ShaderResourceView* mStoneNormalTexSRV;
	ID3D11ShaderResourceView* mBrickNormalTexSRV;

	DirectionalLight mDirLights[3];
	Material mGridMat;
	Material mBoxMat;
	Material mCylinderMat;
	Material mSphereMat;
	Material mSkullMat;

	// 定义从本地空间到世界空间的转换。
	XMFLOAT4X4 mSphereWorld[10];
	XMFLOAT4X4 mCylWorld[10];
	XMFLOAT4X4 mBoxWorld;
	XMFLOAT4X4 mGridWorld;
	XMFLOAT4X4 mSkullWorld;

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

	UINT mSkullIndexCount;

	RenderOptions mRenderOptions;

	Camera mCam;

	POINT mLastMousePos;

};

