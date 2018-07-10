#pragma once
#include <D3DApp.h>
#include <d3dx11effect.h>
#include <d3dcompiler.h>
#include <SimpleMath.h>
#include "MathHelper.h"

using namespace DirectX::SimpleMath;

struct Vertex
{
	XMFLOAT3 Pos;
	XMFLOAT4 Color;
};

class DrawBoxDemo : public D3DApp
{
public:
	DrawBoxDemo(HINSTANCE hinstance);
	virtual ~DrawBoxDemo();

	virtual bool Init() override;
	virtual void OnResize() override;
	virtual void UpdateScene(float dt) override;
	virtual void DrawScene() override;
	virtual void OnMouseDown(WPARAM btnState, int x, int y) override;
	virtual void OnMouseUp(WPARAM btnState, int x, int y) override;
	virtual void OnMouseMove(WPARAM btnState, int x, int y) override;

private:
	void BuildGeometryBuffers();
	void BuildShader();
private:
	ID3D11Buffer * m_pBoxVB;	// ���㻺����
	ID3D11Buffer* m_pBoxIB; // ����������

	ID3D11InputLayout* m_pInputLayout;	// �������벼��


	ID3DX11Effect* m_pFX;	// effect
	ID3DX11EffectTechnique* m_pTech;	// technique
	ID3DX11EffectMatrixVariable* m_pFXWorldViewProj;	// �洢effect�еı���

	XMFLOAT4X4 m_world;
	XMFLOAT4X4 m_view;
	XMFLOAT4X4 m_proj;

	//Matrix m_world;
	//Matrix m_view;
	//Matrix m_proj;

	float m_theta;	//
	float m_phi;	//
	float m_radius;	//�뾶

	POINT m_lastMousePos;

};

