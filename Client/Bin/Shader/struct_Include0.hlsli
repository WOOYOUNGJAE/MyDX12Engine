#ifndef _STRUCT0_
#define _STRUCT0_

struct Material
{
    float3 vAmbient;
    float fShininess;

    float3 vDiffuse;
    float padding0;

    float3 vSpecular;
    float padding1;
};

struct Light
{
    float3 vStrength;
    float fFallOffStart;

    float3 vDir;
    float fFallOffEnd;

    float3 vPos;
    float fSpotPower;
};

struct VSInput_PosNorTex
{
    float4 vPos : POSITION;
    float3 vNormal : NORMAL;
    float2 vUV : TEXCOORD;
};

struct PSInput_Phong
{
    float4 vInPos : SV_POSITION; // Input Pos, VShader의 결과 위치
    float3 vWorldPos : POSITION; // World에서의 위치
    float3 vWorldNormal : NORMAL;
    float2 vUV : TEXCOORD;
};
#endif
