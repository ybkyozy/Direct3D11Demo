#pragma once
#include <D3DApp.h>
#include <d3dx11effect.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <DDSTextureLoader.h>

#include "MathHelper.h"
#include "GeometryGenerator.h"
#include "LightHelper.h"
#include "Camera.h"
#include "Effects.h"
#include "Vertex.h"
#include "Octree.h"


using namespace DirectX;


enum RenderOptions
{
	RenderOptionsBasic = 0,
	RenderOptionsNormalMap = 1,
	RenderOptionsDisplacementMap = 2
};


class AmbientOcclusionDemo : public D3DApp
{
public:
	AmbientOcclusionDemo(HINSTANCE hinstance);
	virtual ~AmbientOcclusionDemo();

	virtual bool Init() override;
	virtual void OnResize() override;
	virtual void UpdateScene(float dt) override;
	virtual void DrawScene() override;
	virtual void OnMouseDown(WPARAM btnState, int x, int y) override;
	virtual void OnMouseUp(WPARAM btnState, int x, int y) override;
	virtual void OnMouseMove(WPARAM btnState, int x, int y) override;

private:
	void BuildVertexAmbientOcclusion(
		std::vector<Vertex::AmbientOcclusion>& vertices,
		const std::vector<UINT>& indices);

	void BuildSkullGeometryBuffers();

private:
	ID3D11Buffer * mSkullVB;
	ID3D11Buffer* mSkullIB;

	XMFLOAT4X4 mSkullWorld;

	UINT mSkullIndexCount;

	Camera mCam;

	POINT mLastMousePos;

};

