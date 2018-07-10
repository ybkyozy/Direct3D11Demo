//***************************************************************************************
// BlurFilter.h by Frank Luna (C) 2011 All Rights Reserved.
//
// Performs a blur operation on the topmost mip level of an input texture.
//***************************************************************************************

#ifndef BLURFILTER_H
#define BLURFILTER_H

#include <Windows.h>
#include <DirectXMath.h>
#include "d3dUtil.h"

class BlurFilter
{
public:
	BlurFilter();
	~BlurFilter();


	ID3D11ShaderResourceView* GetBlurredOutput();

	// 生成高斯模糊权重。
	void SetGaussianWeights(float sigma);

	// 手动指定模糊权重。
	void SetWeights(const float weights[9]);

	///<summary>
	/// 宽度和高度应与输入纹理的尺寸相匹配以模糊。
	/// 可以再次调用Init（）以重新初始化模糊过滤器
	/// 尺寸或格式。
	///</summary>
	void Init(ID3D11Device* device, UINT width, UINT height, DXGI_FORMAT format);

	///<summary>
	/// 模糊输入纹理和模糊次数。 请注意，这会修改输入纹理，而不是它的副本。
	///</summary>
	void BlurInPlace(ID3D11DeviceContext* dc, ID3D11ShaderResourceView* inputSRV, ID3D11UnorderedAccessView* inputUAV, int blurCount);

private:

	UINT mWidth;
	UINT mHeight;
	DXGI_FORMAT mFormat;

	ID3D11ShaderResourceView* mBlurredOutputTexSRV;
	ID3D11UnorderedAccessView* mBlurredOutputTexUAV;
};

#endif // BLURFILTER_H