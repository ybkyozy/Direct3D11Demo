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


class PickingDemo : public D3DApp
{
public:
	PickingDemo(HINSTANCE hinstance);
	virtual ~PickingDemo();

	virtual bool Init() override;
	virtual void OnResize() override;
	virtual void UpdateScene(float dt) override;
	virtual void DrawScene() override;
	virtual void OnMouseDown(WPARAM btnState, int x, int y) override;
	virtual void OnMouseUp(WPARAM btnState, int x, int y) override;
	virtual void OnMouseMove(WPARAM btnState, int x, int y) override;

private:
	void BuildMeshGeometryBuffers();
	void Pick(int sx, int sy);
private:
	ID3D11Buffer * mMeshVB;
	ID3D11Buffer* mMeshIB;

	// Keep system memory copies of the Mesh geometry for picking.
	std::vector<Vertex::Basic32> mMeshVertices;
	std::vector<UINT> mMeshIndices;

	BoundingBox mMeshBox;

	DirectionalLight mDirLights[3];
	Material mMeshMat;
	Material mPickedTriangleMat;

	// Define transformations from local spaces to world space.
	XMFLOAT4X4 mMeshWorld;

	UINT mMeshIndexCount;

	UINT mPickedTriangle;

	Camera mCam;

	POINT mLastMousePos;

};

