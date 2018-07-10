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

#pragma region AmbientOcclusionEffect
class AmbientOcclusionEffect : public Effect
{
public:
	AmbientOcclusionEffect(ID3D11Device* device, const std::wstring& filename);
	~AmbientOcclusionEffect();

	void SetWorldViewProj(CXMMATRIX M) { WorldViewProj->SetMatrix(reinterpret_cast<const float*>(&M)); }

	ID3DX11EffectTechnique* AmbientOcclusionTech;
	ID3DX11EffectMatrixVariable* WorldViewProj;
};
#pragma endregion

class Effects
{
public:
	static void InitAll(ID3D11Device* device);
	static void DestroyAll();

	static AmbientOcclusionEffect* AmbientOcclusionFX;
};