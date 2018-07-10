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
#include "RenderStates.h"
#include "Camera.h"


using namespace DirectX;

struct InstancedData
{
	XMFLOAT4X4 World;
	XMFLOAT4 Color;
};

class InstancingAndCullingDemo : public D3DApp
{
public:
	InstancingAndCullingDemo(HINSTANCE hinstance);
	virtual ~InstancingAndCullingDemo();

	virtual bool Init() override;
	virtual void OnResize() override;
	virtual void UpdateScene(float dt) override;
	virtual void DrawScene() override;
	virtual void OnMouseDown(WPARAM btnState, int x, int y) override;
	virtual void OnMouseUp(WPARAM btnState, int x, int y) override;
	virtual void OnMouseMove(WPARAM btnState, int x, int y) override;

private:
	void BuildSkullGeometryBuffers();
	void BuildInstancedBuffer();
private:
	ID3D11Buffer * mSkullVB;
	ID3D11Buffer* mSkullIB;
	ID3D11Buffer* mInstancedBuffer;

	// Bounding box of the skull.
	BoundingBox mSkullBox;
	BoundingFrustum mCamFrustum;

	UINT mVisibleObjectCount;

	// Keep a system memory copy of the world matrices for culling.
	std::vector<InstancedData> mInstancedData;

	bool mFrustumCullingEnabled;

	DirectionalLight mDirLights[3];
	Material mSkullMat;

	// Define transformations from local spaces to world space.
	XMFLOAT4X4 mSkullWorld;

	UINT mSkullIndexCount;

	Camera mCam;

	POINT mLastMousePos;

};

