#pragma once

#include "D3DUtil.h"

class Effect
{
public:
	Effect(ID3D11Device* device, const std::wstring& fileName);
	virtual ~Effect();

protected:
	ID3DX11Effect * mFX;
};


class BasicEffect : public Effect
{
public:
	BasicEffect(ID3D11Device* device, const std::wstring& filename);
	~BasicEffect();

	void SetWorldViewProj(CXMMATRIX M) { WorldViewProj->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetWorld(CXMMATRIX M) { World->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetWorldInvTranspose(CXMMATRIX M) { WorldInvTranspose->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetEyePosW(const XMFLOAT3& v) { EyePosW->SetRawValue(&v, 0, sizeof(XMFLOAT3)); }
	void SetDirLights(const DirectionalLight* lights) { DirLights->SetRawValue(lights, 0, 3 * sizeof(DirectionalLight)); }
	void SetMaterial(const Material& mat) { Mat->SetRawValue(&mat, 0, sizeof(Material)); }

	ID3DX11EffectTechnique* Light1Tech;
	ID3DX11EffectTechnique* Light2Tech;
	ID3DX11EffectTechnique* Light3Tech;

	ID3DX11EffectMatrixVariable* WorldViewProj;
	ID3DX11EffectMatrixVariable* World;
	ID3DX11EffectMatrixVariable* WorldInvTranspose;
	ID3DX11EffectVectorVariable* EyePosW;
	ID3DX11EffectVariable* DirLights;
	ID3DX11EffectVariable* Mat;
};


class Effects
{
public:
	static void InitAll(ID3D11Device* device);
	static void DestroyAll();

	static BasicEffect* BasicFX;
};