
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
    float3 vPos : POSITION;
    float3 vNormal : NORMAL;
    float2 vUV : TEXCOORD0;
};

struct PSInput0
{
    float4 vInPos : SV_POSITION; // Input Pos
    float3 vWorldPos : POSITION;
    float3 vWorldNormal : NORMAL;
    float2 vUV : TEXCOORD;
};
