//***************************************************************************************
// BlurFilter.cpp by Frank Luna (C) 2011 All Rights Reserved.
//***************************************************************************************

#include "BlurFilter.h"
#include "Effects.h"

BlurFilter::BlurFilter()
  : mBlurredOutputTexSRV(0), mBlurredOutputTexUAV(0)
{
}

BlurFilter::~BlurFilter()
{
	ReleaseCOM(mBlurredOutputTexSRV);
	ReleaseCOM(mBlurredOutputTexUAV);
}

ID3D11ShaderResourceView* BlurFilter::GetBlurredOutput()
{
	return mBlurredOutputTexSRV;
}

void BlurFilter::SetGaussianWeights(float sigma)
{
	float d = 2.0f*sigma*sigma;

	float weights[9];
	float sum = 0.0f;
	for(int i = 0; i < 8; ++i)
	{
		float x = (float)i;
		weights[i] = expf(-x*x/d);

		sum += weights[i];
	}

	// 除以总数，所有权重加起来为1.0。
	for(int i = 0; i < 8; ++i)
	{
		weights[i] /= sum;
	}

	Effects::BlurFX->SetWeights(weights);
}

void BlurFilter::SetWeights(const float weights[9])
{
	Effects::BlurFX->SetWeights(weights);
}

void BlurFilter::Init(ID3D11Device* device, UINT width, UINT height, DXGI_FORMAT format)
{
	// Start fresh.
	ReleaseCOM(mBlurredOutputTexSRV);
	ReleaseCOM(mBlurredOutputTexUAV);

	mWidth = width;
	mHeight = height;
	mFormat = format;

	// 注意：压缩格式不支持无序访问视图 UAV.  我们将得到错误 如:
	// ERROR: ID3D11Device::CreateTexture2D: The format (0x4d, BC3_UNORM) 
	// cannot be bound as an UnorderedAccessView, or cast to a format that
	// could be bound as an UnorderedAccessView.  Therefore this format 
	// does not support D3D11_BIND_UNORDERED_ACCESS.

	D3D11_TEXTURE2D_DESC blurredTexDesc;
	blurredTexDesc.Width     = width;
	blurredTexDesc.Height    = height;
    blurredTexDesc.MipLevels = 1;
    blurredTexDesc.ArraySize = 1;
	blurredTexDesc.Format    = format;
	blurredTexDesc.SampleDesc.Count   = 1;
	blurredTexDesc.SampleDesc.Quality = 0;
    blurredTexDesc.Usage     = D3D11_USAGE_DEFAULT;
    blurredTexDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
    blurredTexDesc.CPUAccessFlags = 0;
    blurredTexDesc.MiscFlags      = 0;

	ID3D11Texture2D* blurredTex = 0;
	HR(device->CreateTexture2D(&blurredTexDesc, 0, &blurredTex));

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	HR(device->CreateShaderResourceView(blurredTex, &srvDesc, &mBlurredOutputTexSRV));

	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	uavDesc.Format = format;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Texture2D.MipSlice = 0;
	HR(device->CreateUnorderedAccessView(blurredTex, &uavDesc, &mBlurredOutputTexUAV));

	// 视图保存对纹理的引用，以便我们可以释放我们的引用。
	ReleaseCOM(blurredTex);
}

void BlurFilter::BlurInPlace(ID3D11DeviceContext* dc, 
							 ID3D11ShaderResourceView* inputSRV, 
	                         ID3D11UnorderedAccessView* inputUAV,
							 int blurCount)
{
	//
	// 运行计算着色器以模糊离屏纹理。
	// 

	for(int i = 0; i < blurCount; ++i)
	{
		// 水平模糊过程
		D3DX11_TECHNIQUE_DESC techDesc;
		Effects::BlurFX->HorzBlurTech->GetDesc( &techDesc );
		for(UINT p = 0; p < techDesc.Passes; ++p)
		{
			Effects::BlurFX->SetInputMap(inputSRV);
			Effects::BlurFX->SetOutputMap(mBlurredOutputTexUAV);
			Effects::BlurFX->HorzBlurTech->GetPassByIndex(p)->Apply(0, dc);

			// 我们需要分派多少组以覆盖一行像素，其中每行都是
			// 组涵盖了256个像素（256是在ComputeShader中定义的）。
			UINT numGroupsX = (UINT)ceilf(mWidth / 256.0f);
			dc->Dispatch(numGroupsX, mHeight, 1);
		}
	
		// 从CS取消绑定输入纹理以获得良好的内务管理。
		ID3D11ShaderResourceView* nullSRV[1] = { 0 };
		dc->CSSetShaderResources( 0, 1, nullSRV );

		// 从计算着色器中取消绑定输出（我们将在下一个传递中使用该输出作为输入，
		// 并且资源不能同时是输出和输入。
		ID3D11UnorderedAccessView* nullUAV[1] = { 0 };
		dc->CSSetUnorderedAccessViews( 0, 1, nullUAV, 0 );
	
		// 垂直模糊过程
		Effects::BlurFX->VertBlurTech->GetDesc( &techDesc );
		for(UINT p = 0; p < techDesc.Passes; ++p)
		{
			Effects::BlurFX->SetInputMap(mBlurredOutputTexSRV);
			Effects::BlurFX->SetOutputMap(inputUAV);
			Effects::BlurFX->VertBlurTech->GetPassByIndex(p)->Apply(0, dc);

			//我们需要分派多少个组以覆盖像素列，其中每列都是
			//组包含256个像素（256是在ComputeShader中定义的）。
			UINT numGroupsY = (UINT)ceilf(mHeight / 256.0f);
			dc->Dispatch(mWidth, numGroupsY, 1);
		}
	
		dc->CSSetShaderResources( 0, 1, nullSRV );
		dc->CSSetUnorderedAccessViews( 0, 1, nullUAV, 0 );
	}

	// 禁用计算着色器。
	dc->CSSetShader(0, 0, 0);
}
