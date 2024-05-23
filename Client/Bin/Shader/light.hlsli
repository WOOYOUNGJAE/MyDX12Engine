#ifndef _LIGHT_
#define _LIGHT_

#include "struct_Include0.hlsli"


float3 BlinnPhong(float3 lightStrength, float3 lightVec, float3 normal,
                   float3 toEye, Material mat)
{
    float3 halfway = normalize(toEye + lightVec);
    float hdotn = dot(halfway, normal);
    float3 vSpecular = mat.vSpecular * pow(max(hdotn, 0.f), mat.fShininess);

    return mat.vAmbient + (mat.vDiffuse + vSpecular) * lightStrength;
}

float3 ComputeDirectionalLight(Light L, Material mat, float3 normal,
                                float3 toEye)
{
    float3 vLightVec = -L.vDir;

    float ndotl = max(dot(vLightVec, normal), 0.f);
    float3 vLightStrength = L.vStrength * ndotl;

    return BlinnPhong(vLightStrength, vLightVec, normal, toEye, mat);
}

#endif
