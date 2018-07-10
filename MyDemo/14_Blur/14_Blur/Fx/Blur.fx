//=============================================================================
// Blur.fx by Frank Luna (C) 2011 All Rights Reserved.
//
// Performs a separable blur with a blur radius of 5.  
//=============================================================================

cbuffer cbSettings
{
	//权重矩阵
	float gWeights[11] = 
	{
		0.05f, 0.05f, 0.1f, 0.1f, 0.1f, 0.2f, 0.1f, 0.1f, 0.1f, 0.05f, 0.05f,
	};
};

cbuffer cbFixed
{
	static const int gBlurRadius = 5;
};

Texture2D gInput;
RWTexture2D<float4> gOutput;

#define N 256
#define CacheSize (N + 2*gBlurRadius)
groupshared float4 gCache[CacheSize];

//numthreads(N, 1, 1)，代表着线程的数量，可以是多维的
//groupThreadID，线程在线程组中的ID
//dispatchThreadID，在一次dispatch线程中的ID

//水平模糊
[numthreads(N, 1, 1)]
void HorzBlurCS(int3 groupThreadID : SV_GroupThreadID,
				int3 dispatchThreadID : SV_DispatchThreadID)
{
	//
	// 填充本地线程存储以减少带宽。 模糊
	// N个像素，我们需要加载N + 2 * BlurRadius像素
	// 由于模糊半径。
	//
	
	// 该线程组运行N个线程。 要获得额外的2 * BlurRadius像素，
	// 有2 * BlurRadius线程采样一个额外的像素。
	if(groupThreadID.x < gBlurRadius)
	{
		// 限制出现在图像边界处的采样。
		int x = max(dispatchThreadID.x - gBlurRadius, 0);
		gCache[groupThreadID.x] = gInput[int2(x, dispatchThreadID.y)];
	}
	if(groupThreadID.x >= N-gBlurRadius)
	{
		// 限制出现在图像边界处的采样。
		int x = min(dispatchThreadID.x + gBlurRadius, gInput.Length.x-1);
		gCache[groupThreadID.x+2*gBlurRadius] = gInput[int2(x, dispatchThreadID.y)];
	}

	// 限制出现在图像边界处的采样。
	gCache[groupThreadID.x+gBlurRadius] = gInput[min(dispatchThreadID.xy, gInput.Length.xy-1)];

	// 等待所有线程完成。
	GroupMemoryBarrierWithGroupSync();
	
	//
	// 现在模糊每个像素。
	//

	float4 blurColor = float4(0, 0, 0, 0);
	
	[unroll]
	for(int i = -gBlurRadius; i <= gBlurRadius; ++i)
	{
		int k = groupThreadID.x + gBlurRadius + i;
		
		blurColor += gWeights[i+gBlurRadius]*gCache[k];
	}
	
	gOutput[dispatchThreadID.xy] = blurColor;
}

[numthreads(1, N, 1)]
void VertBlurCS(int3 groupThreadID : SV_GroupThreadID,
				int3 dispatchThreadID : SV_DispatchThreadID)
{
	//
	// 填充本地线程存储以减少带宽。 模糊
	// N个像素，我们需要加载N + 2 * BlurRadius像素
	// 由于模糊半径。
	//
	
	// 该线程组运行N个线程。 要获得额外的2 * BlurRadius像素，
	// 有2 * BlurRadius线程采样一个额外的像素。
	if(groupThreadID.y < gBlurRadius)
	{
		// 限制出现在图像边界处的采样。
		int y = max(dispatchThreadID.y - gBlurRadius, 0);
		gCache[groupThreadID.y] = gInput[int2(dispatchThreadID.x, y)];
	}
	if(groupThreadID.y >= N-gBlurRadius)
	{
		// 限制出现在图像边界处的采样。
		int y = min(dispatchThreadID.y + gBlurRadius, gInput.Length.y-1);
		gCache[groupThreadID.y+2*gBlurRadius] = gInput[int2(dispatchThreadID.x, y)];
	}
	
	// 限制出现在图像边界处的采样。
	gCache[groupThreadID.y+gBlurRadius] = gInput[min(dispatchThreadID.xy, gInput.Length.xy-1)];


	// 等待所有线程完成。
	GroupMemoryBarrierWithGroupSync();
	
	//
	// 现在模糊每个像素。
	//

	float4 blurColor = float4(0, 0, 0, 0);
	
	[unroll]
	for(int i = -gBlurRadius; i <= gBlurRadius; ++i)
	{
		int k = groupThreadID.y + gBlurRadius + i;
		
		blurColor += gWeights[i+gBlurRadius]*gCache[k];
	}
	
	gOutput[dispatchThreadID.xy] = blurColor;
}

technique11 HorzBlur
{
    pass P0
    {
		SetVertexShader( NULL );
        SetPixelShader( NULL );
		SetComputeShader( CompileShader( cs_5_0, HorzBlurCS() ) );
    }
}

technique11 VertBlur
{
    pass P0
    {
		SetVertexShader( NULL );
        SetPixelShader( NULL );
		SetComputeShader( CompileShader( cs_5_0, VertBlurCS() ) );
    }
}
