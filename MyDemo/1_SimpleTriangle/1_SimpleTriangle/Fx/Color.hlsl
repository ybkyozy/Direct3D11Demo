float4 VS(float3 pos : POSITION):SV_Position
{
    return float4(pos, 1.0f);
}

float4 PS(float4 posIn : SV_Position) : SV_Target
{
    return float4(1.0f, 0.0f, 0.0f, 1.0f);
}

technique11 ColorTech
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetPixelShader(CompileShader(ps_5_0, PS()));
    }
}