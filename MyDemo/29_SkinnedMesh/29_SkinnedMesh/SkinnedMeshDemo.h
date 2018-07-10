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
#include "ShadowMap.h"
#include "Ssao.h"
#include "TextureMgr.h"
#include "BasicModel.h"
#include "SkinnedModel.h"

using namespace DirectX;



class MeshViewDemo : public D3DApp
{
public:
	MeshViewDemo(HINSTANCE hinstance);
	virtual ~MeshViewDemo();

	virtual bool Init() override;
	virtual void OnResize() override;
	virtual void UpdateScene(float dt) override;
	virtual void DrawScene() override;
	virtual void OnMouseDown(WPARAM btnState, int x, int y) override;
	virtual void OnMouseUp(WPARAM btnState, int x, int y) override;
	virtual void OnMouseMove(WPARAM btnState, int x, int y) override;

private:
	void DrawSceneToSsaoNormalDepthMap();
	void DrawSceneToShadowMap();
	void DrawScreenQuad(ID3D11ShaderResourceView* srv);
	void BuildShadowTransform();
	void BuildShapeGeometryBuffers();
	void BuildSkullGeometryBuffers();
	void BuildScreenQuadGeometryBuffers();

private:
	TextureMgr mTexMgr;

	Sky* mSky;

	SkinnedModel* mCharacterModel;
	SkinnedModelInstance mCharacterInstance1;
	SkinnedModelInstance mCharacterInstance2;

	ID3D11Buffer* mShapesVB;
	ID3D11Buffer* mShapesIB;

	ID3D11Buffer* mSkullVB;
	ID3D11Buffer* mSkullIB;

	ID3D11Buffer* mSkySphereVB;
	ID3D11Buffer* mSkySphereIB;

	ID3D11Buffer* mScreenQuadVB;
	ID3D11Buffer* mScreenQuadIB;

	ID3D11ShaderResourceView* mStoneTexSRV;
	ID3D11ShaderResourceView* mBrickTexSRV;

	ID3D11ShaderResourceView* mStoneNormalTexSRV;
	ID3D11ShaderResourceView* mBrickNormalTexSRV;

	BoundingSphere mSceneBounds;

	static const int SMapSize = 2048;
	ShadowMap* mSmap;
	XMFLOAT4X4 mLightView;
	XMFLOAT4X4 mLightProj;
	XMFLOAT4X4 mShadowTransform;

	Ssao* mSsao;

	float mLightRotationAngle;
	XMFLOAT3 mOriginalLightDir[3];
	DirectionalLight mDirLights[3];
	Material mGridMat;
	Material mBoxMat;
	Material mCylinderMat;
	Material mSphereMat;
	Material mSkullMat;

	// Define transformations from local spaces to world space.
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

	Camera mCam;

	POINT mLastMousePos;

};

