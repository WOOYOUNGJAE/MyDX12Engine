//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#ifndef RAYTRACING_DEFINES
#define RAYTRACING_DEFINES

#define MAX_RAY_RECURSION_DEPTH 2
#define NUM_OBJECTS 3 // TODO 개수 TEMP
//#define MAX_RAY_RECURSION_DEPTH 3    // ~ primary rays + reflections + shadow rays from reflected geometry.
#ifdef HLSL
#include "HlslCompat.h"
#else


typedef float3 XMFLOAT3;
typedef float4 XMFLOAT4;
typedef float4 XMVECTOR;
typedef float4x4 XMMATRIX;
typedef uint UINT;

// Shader will use byte encoding to access indices.
typedef UINT16 Index;
#endif

struct DXR_Scene_CB
{
	float4x4 viewProjectionInv;
    float4 cameraPosition;
    float4 lightPosition;
    float4 lightAmbientColor;
    float4 lightDiffuseColor;
};

struct OBJECT_CB_STATIC
{
    UINT startIndex_in_IB_SRV; // Index Buffer ShaderResourceView의 시작 인덱스
    UINT startIndex_in_VB_SRV;
    float2 padding;
    XMFLOAT4 albedo;
};
struct OBJECT_CB_STATIC_Arr
{
    OBJECT_CB_STATIC object_cb_static[NUM_OBJECTS];
};

struct OBJECT_CB_DYNAMIC
{
    float4x4 InvTranspose;
};

struct OBJECT_CB_DYNAMIC_Arr
{
    OBJECT_CB_DYNAMIC object_cb_dynamic[NUM_OBJECTS];
};

struct VertexPositionNormalColorTexture
{
    float3 position;
    float3 normal;
    float4 color;
    float2 uv;
};

struct MyRay
{
    float3 origin;
    float3 direction;
};
struct MyRayPayload
{
    XMFLOAT4 color;
    UINT   recursionDepth;
};

// Ray types traced
namespace RayType {
    enum Enum {
        Radiance = 0,   // ~ Primary, reflected camera/view rays calculating color for each hit.
        //Shadow,         // ~ Shadow/visibility rays, only testing for occlusion
        Count
    };
}

namespace TraceRayParameters
{
    static const UINT InstanceMask = ~0;   // Everything is visible.
    namespace HitGroup {
        static const UINT Offset[RayType::Count] =
        {
            0, // Radiance ray
            //1  // Shadow ray
        };
        static const UINT GeometryStride = RayType::Count;
    }
    namespace MissShader {
        static const UINT Offset[RayType::Count] =
        {
            0, // Radiance ray
            //1  // Shadow ray
        };
    }
}
#endif // RAYTRACING_DEFINES