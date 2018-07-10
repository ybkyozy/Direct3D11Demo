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

using namespace DirectX;

enum RenderOptions
{
	Lighting = 0,
	Textures = 1,
	TexturesAndFog = 2
};


class MirrorDemo : public D3DApp
{
public:
	MirrorDemo(HINSTANCE hinstance);
	virtual ~MirrorDemo();

	virtual bool Init() override;
	virtual void OnResize() override;
	virtual void UpdateScene(float dt) override;
	virtual void DrawScene() override;
	virtual void OnMouseDown(WPARAM btnState, int x, int y) override;
	virtual void OnMouseUp(WPARAM btnState, int x, int y) override;
	virtual void OnMouseMove(WPARAM btnState, int x, int y) override;

private:
	void BuildRoomGeometryBuffers();
	void BuildSkullGeometryBuffers();
private:
	ID3D11Buffer * mRoomVB;

	ID3D11Buffer* mSkullVB;
	ID3D11Buffer* mSkullIB;

	ID3D11ShaderResourceView* mFloorDiffuseMapSRV;
	ID3D11ShaderResourceView* mWallDiffuseMapSRV;
	ID3D11ShaderResourceView* mMirrorDiffuseMapSRV;

	DirectionalLight mDirLights[3];
	Material mRoomMat;
	Material mSkullMat;
	Material mMirrorMat;
	Material mShadowMat;

	XMFLOAT4X4 mRoomWorld;
	XMFLOAT4X4 mSkullWorld;

	UINT mSkullIndexCount;
	XMFLOAT3 mSkullTranslation;

	XMFLOAT4X4 mView;
	XMFLOAT4X4 mProj;

	RenderOptions mRenderOptions;

	XMFLOAT3 mEyePosW;

	float mTheta;
	float mPhi;
	float mRadius;

	POINT mLastMousePos;

};

