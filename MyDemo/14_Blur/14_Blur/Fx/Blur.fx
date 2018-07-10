//=============================================================================
// Blur.fx by Frank Luna (C) 2011 All Rights Reserved.
//
// Performs a separable blur with a blur radius of 5.  
//=============================================================================

cbuffer cbSettings
{
	//Ȩ�ؾ���
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

//numthreads(N, 1, 1)���������̵߳������������Ƕ�ά��
//groupThreadID���߳����߳����е�ID
//dispatchThreadID����һ��dispatch�߳��е�ID

//ˮƽģ��
[numthreads(N, 1, 1)]
void HorzBlurCS(int3 groupThreadID : SV_GroupThreadID,
				int3 dispatchThreadID : SV_DispatchThreadID)
{
	//
	// ��䱾���̴߳洢�Լ��ٴ��� ģ��
	// N�����أ�������Ҫ����N + 2 * BlurRadius����
	// ����ģ���뾶��
	//
	
	// ���߳�������N���̡߳� Ҫ��ö����2 * BlurRadius���أ�
	// ��2 * BlurRadius�̲߳���һ����������ء�
	if(groupThreadID.x < gBlurRadius)
	{
		// ���Ƴ�����ͼ��߽紦�Ĳ�����
		int x = max(dispatchThreadID.x - gBlurRadius, 0);
		gCache[groupThreadID.x] = gInput[int2(x, dispatchThreadID.y)];
	}
	if(groupThreadID.x >= N-gBlurRadius)
	{
		// ���Ƴ�����ͼ��߽紦�Ĳ�����
		int x = min(dispatchThreadID.x + gBlurRadius, gInput.Length.x-1);
		gCache[groupThreadID.x+2*gBlurRadius] = gInput[int2(x, dispatchThreadID.y)];
	}

	// ���Ƴ�����ͼ��߽紦�Ĳ�����
	gCache[groupThreadID.x+gBlurRadius] = gInput[min(dispatchThreadID.xy, gInput.Length.xy-1)];

	// �ȴ������߳���ɡ�
	GroupMemoryBarrierWithGroupSync();
	
	//
	// ����ģ��ÿ�����ء�
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
	// ��䱾���̴߳洢�Լ��ٴ��� ģ��
	// N�����أ�������Ҫ����N + 2 * BlurRadius����
	// ����ģ���뾶��
	//
	
	// ���߳�������N���̡߳� Ҫ��ö����2 * BlurRadius���أ�
	// ��2 * BlurRadius�̲߳���һ����������ء�
	if(groupThreadID.y < gBlurRadius)
	{
		// ���Ƴ�����ͼ��߽紦�Ĳ�����
		int y = max(dispatchThreadID.y - gBlurRadius, 0);
		gCache[groupThreadID.y] = gInput[int2(dispatchThreadID.x, y)];
	}
	if(groupThreadID.y >= N-gBlurRadius)
	{
		// ���Ƴ�����ͼ��߽紦�Ĳ�����
		int y = min(dispatchThreadID.y + gBlurRadius, gInput.Length.y-1);
		gCache[groupThreadID.y+2*gBlurRadius] = gInput[int2(dispatchThreadID.x, y)];
	}
	
	// ���Ƴ�����ͼ��߽紦�Ĳ�����
	gCache[groupThreadID.y+gBlurRadius] = gInput[min(dispatchThreadID.xy, gInput.Length.xy-1)];


	// �ȴ������߳���ɡ�
	GroupMemoryBarrierWithGroupSync();
	
	//
	// ����ģ��ÿ�����ء�
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
