//=============================================================================
// Sky.fx by Frank Luna (C) 2011 All Rights Reserved.
//
// Effect used to shade sky dome.
//=============================================================================

cbuffer cbPerFrame
{
	float4x4 gWorldViewProj;
};
 
// Nonnumeric values cannot be added to a cbuffer.
TextureCube gCubeMap;

SamplerState samTriLinearSam
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
};

struct VertexIn
{
	float3 PosL : POSITION;
};

struct VertexOut
{
	float4 PosH : SV_POSITION;
    float3 PosL : POSITION;
};
 
VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	
	// ���� z = w ���� z/w = 1 (����ղ��������Զƽ��).
	vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj).xyww;
	
	// ʹ�ñ��ض���λ����Ϊ��������ͼ����������
	vout.PosL = vin.PosL;
	
	return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
	return gCubeMap.Sample(samTriLinearSam, pin.PosL);
}

RasterizerState NoCull
{
    CullMode = None;
};

DepthStencilState LessEqualDSS
{
	// ȷ����Ⱥ�����<=����������<��
	// ���������Ȼ��������������z = 1��NDC�����Ĺ�һ�����ֵ����Ϊ1
	// ����Ȳ���ʧ�ܡ�
    DepthFunc = LESS_EQUAL;
};

technique11 SkyTech
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS() ) );
        
        SetRasterizerState(NoCull);
        SetDepthStencilState(LessEqualDSS, 0);
    }
}
