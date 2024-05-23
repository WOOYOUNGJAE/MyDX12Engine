// light 적용된 쉐이더
#include "light.hlsli"
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

Texture2D g_texture : register(t0);
SamplerState g_sampler : register(s0);


PSInput_Phong VS(VSInput_PosNorTex vsIn)
{
    PSInput_Phong result;

    result.vWorldPos = vsIn.vPos; // 월드 위치 따로 저장
    result.vInPos = mul(mul(mul(vsIn.vPos, g_worldMat), g_viewMat), g_projMat);
    result.vUV = vsIn.vUV;

    float4 worldNormal = float4(vsIn.vNormal, 0);
    result.vWorldNormal = mul(worldNormal, g_invTranspose).xyz;
    result.vWorldNormal = normalize(result.vWorldNormal);

    return result;

    // pos는 모델변환된 매트릭스를 받고 normal은 쉐이더 내에서 변환해야 함
    //result.vWorldNormal = mul(vsIn.vNormal, )

    //result.position = mul(position, g_worldMat);
    //result.position = mul(mul(mul(vsIn.vPos, g_worldMat), g_viewMat), g_projMat);
    //result.position = position;

    return result;
}

float4 PS(PSInput_Phong psIn) : SV_TARGET
{
    float3 vEyeWorld = g_viewMat[3].xyz;
    float3 vToEye = normalize(vEyeWorld - psIn.vWorldPos);

    float3 resultColor = float3(0.0, 0.0, 0.0);
    
    resultColor += ComputeDirectionalLight(g_light, g_material, psIn.vWorldNormal, vToEye);
    
    return g_texture.Sample(g_sampler, psIn.vUV);
    return float4(resultColor, 1.0) * g_texture.Sample(g_sampler, psIn.vUV);
}
