#include "struct_Include0.hlsli"

cbuffer Object_Cb : register(b0)
{
    float4x4 g_worldMat;
    float4x4 g_invTranspose;
    Material g_material;
    float4 padding0[5];
};
cbuffer Pass_Cb : register(b1)
{
    float4x4 g_viewMat;
    float4x4 g_projMat;
    Light g_light;
    float4 padding1[5];
};
struct VSInput
{
    float4 vPos : POSITION;
    float3 vNormal : NORMAL;
    float2 vUV : TEXCOORD;
};
struct PSInput
{
    float4 vInPos : SV_POSITION;
    float4 vPosWorld : POSITION;
};

TextureCube g_textureCube : register(t0);
SamplerState g_sampler : register(s0);

PSInput VS(VSInput vsIn)
{
    PSInput result;

    result.vPosWorld = vsIn.vPos; // 텍스쳐 좌표로 활용
    
    // viewMat와 projMat를 적용하여 카메라 시점에 따라 픽셀 색은 다르게 나오도록
    result.vInPos = mul(mul(mul(vsIn.vPos, g_worldMat), g_viewMat), g_projMat);

    return result;
}

float4 PS(PSInput psIn) : SV_TARGET
{
    return g_textureCube.Sample(g_sampler, psIn.vPosWorld.xyz);
}