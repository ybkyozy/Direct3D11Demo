#include "Effects.h"
#include <d3dcompiler.h>

Effect::Effect(ID3D11Device* device, const std::wstring& fileName)
{
	DWORD shaderFlags = 0;
#if defined( DEBUG ) || defined( _DEBUG )
	shaderFlags |= D3DCOMPILE_DEBUG;
	shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
	ID3DBlob* compilationMsgs = 0;
	HR(D3DX11CompileEffectFromFile(fileName.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, shaderFlags, 0, device, &mFX, &compilationMsgs));
	if (compilationMsgs != 0)
	{
		OutputDebugStringA((char*)compilationMsgs->GetBufferPointer());
		ReleaseCOM(compilationMsgs);
	}

	//HR(D3DX11CreateEffectFromFile(fileName.c_str(), 0, device, &mFX));
}


Effect::~Effect()
{
	ReleaseCOM(mFX);
}

#pragma region AmbientOcclusionEffect
AmbientOcclusionEffect::AmbientOcclusionEffect(ID3D11Device* device, const std::wstring& filename)
	: Effect(device, filename)
{
	AmbientOcclusionTech = mFX->GetTechniqueByName("AmbientOcclusion");

	WorldViewProj = mFX->GetVariableByName("gWorldViewProj")->AsMatrix();
}

AmbientOcclusionEffect::~AmbientOcclusionEffect()
{
}
#pragma endregion

///////////////////////////////////////////////////////////////


AmbientOcclusionEffect* Effects::AmbientOcclusionFX = 0;


void Effects::InitAll(ID3D11Device* device)
{
	AmbientOcclusionFX = new AmbientOcclusionEffect(device, L"FX/AmbientOcclusion.fx");
}


void Effects::DestroyAll()
{
	SafeDelete(AmbientOcclusionFX);
}




