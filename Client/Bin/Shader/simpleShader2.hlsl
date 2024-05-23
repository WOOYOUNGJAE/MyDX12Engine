
struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

Texture2D g_texture : register(t0);
cbuffer Object_Cb : register(b0)
{
    float4x4 g_worldMat;
    float4 padding0[48];
};
cbuffer Pass_Cb : register(b1)
{
    float4x4 g_viewMat;
    float4x4 g_projMat;
    float4 padding1[32];
};

SamplerState g_sampler : register(s0);

struct VSInput
{
    float4 vPos : POSITION;
    float3 vNormal : NORMAL;
    float2 vUV : TEXCOORD;
};

PSInput VS(VSInput vsIn)
{
    PSInput result;

    //result.position = mul(position, g_worldMat);
    result.position = mul(mul(mul(vsIn.vPos, g_worldMat), g_viewMat), g_projMat);
    //result.position = position;
    result.uv = vsIn.vUV;

    return result;
}

float4 PS(PSInput input) : SV_TARGET
{
    return g_texture.Sample(g_sampler, input.uv);
}
