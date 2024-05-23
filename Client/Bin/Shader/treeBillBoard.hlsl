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

struct VSInput
{
    float4 vPos : POSITION;
    float2 width : SIZE;
};

struct GSInput
{
    float4 pos : SV_POSITION; // Screen position
    float2 width : SIZE;
};

struct PSInput
{
    float4 posProj : SV_POSITION; // Screen position
    float2 texCoord : TEXCOORD;
    uint primID : SV_PrimitiveID; // for TexArray index
};

GSInput VS(VSInput vsIn)
{
    GSInput output;

    output.pos = vsIn.vPos;
    output.width = vsIn.width;

    return output;
}

[maxvertexcount(4)]
void GS(point GSInput gsIn[1], uint primID : SV_PrimitiveID,
                              inout TriangleStream<PSInput> outputStream)
{
    float halfWidth = gsIn[0].width.x * 0.5f;

    PSInput output;
    // Triangle Strip 이라 좌하단 삼각형 + 점 한개 = 삼각형 2개

    output.posProj = gsIn[0].pos + float4(-halfWidth, -halfWidth, 0.f, 0.f);
    output.posProj = mul(output.posProj, g_worldMat);
    output.posProj = mul(output.posProj, g_viewMat);
    output.posProj = mul(output.posProj, g_projMat);
    output.texCoord = float2(0.f, 1.f);
    output.primID = primID;

    outputStream.Append(output);

    output.posProj = gsIn[0].pos + float4(-halfWidth, halfWidth, 0.f, 0.f);
    output.posProj = mul(output.posProj, g_worldMat);
    output.posProj = mul(output.posProj, g_viewMat);
    output.posProj = mul(output.posProj, g_projMat);
    output.texCoord = float2(0.f, 0.f);
    output.primID = primID;

    outputStream.Append(output);

    output.posProj = gsIn[0].pos + float4(halfWidth, -halfWidth, 0.f, 0.f);
    output.posProj = mul(output.posProj, g_worldMat);
    output.posProj = mul(output.posProj, g_viewMat);
    output.posProj = mul(output.posProj, g_projMat);
    output.texCoord = float2(1.f, 1.f);
    output.primID = primID;

    outputStream.Append(output);

    output.posProj = gsIn[0].pos + float4(halfWidth, halfWidth, 0.f, 0.f);
    output.posProj = mul(output.posProj, g_worldMat);
    output.posProj = mul(output.posProj, g_viewMat);
    output.posProj = mul(output.posProj, g_projMat);
    output.texCoord = float2(1.f, 0.f);
    output.primID = primID;

    outputStream.Append(output);
}

float4 PS(PSInput psIn) : SV_TARGET
{
    // uvw 중 w는 TexArray 의 Index
    //float3 uvw = float3(psIn.texCoord, float(psIn.primID % 5));
    float3 uvw = float3(psIn.texCoord, float(psIn.primID % 5));

    float4 outputColor = g_texture.Sample(g_sampler, uvw.xy); // TODO TexArray로
    clip(outputColor.a - 0.9f);
    return outputColor;
}