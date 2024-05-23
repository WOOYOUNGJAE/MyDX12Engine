#ifndef RAYTRACING_FUNCTIONS
#define RAYTRACING_FUNCTIONS


#include "RaytracingDefines.h"
typedef BuiltInTriangleIntersectionAttributes MyAttributes;

// offsetBytes : 삼각형의 첫 인덱스 원소의 offset
uint3 Load3x16BitIndices(ByteAddressBuffer Indices,uint offsetBytes)
{
    uint3 indices;

    // ByteAdressBuffer loads must be aligned at a 4 byte boundary.
    // Since we need to read three 16 bit indices: { 0, 1, 2 } 
    // aligned at a 4 byte boundary as: { 0 1 } { 2 0 } { 1 2 } { 0 1 } ...
    // we will load 8 bytes (~ 4 indices { a b | c d }) to handle two possible index triplet layouts,
    // based on first index's offsetBytes being aligned at the 4 byte boundary or not:
    //  Aligned:     { 0 1 | 2 - }
    //  Not aligned: { - 0 | 1 2 }
    const uint dwordAlignedOffset = offsetBytes & ~3; // offsetBytes를 4의 배수로 만들기
    // 오프셋 기반 2개 int 로드
    // int 2개를 로드했지만, UINT16으로 저장했기 때문에 실질적으론 4개가 로드 됨
    const uint2 four16BitIndices = Indices.Load2(dwordAlignedOffset);
 
    // Aligned: { 0 1 | 2 - } => retrieve first three 16bit indices
    if (dwordAlignedOffset == offsetBytes) // 인자로 들어온 인덱스 원소 첫 오프셋이 4의 배수라면
    {
        indices.x = four16BitIndices.x & 0xffff; //32비트 중 앞의 16비트 뽑아옴
        indices.y = (four16BitIndices.x >> 16) & 0xffff; // 32비트 중 뒤 16비트 뽑아옴
        indices.z = four16BitIndices.y & 0xffff;
    }
    else // Not aligned: { - 0 | 1 2 } => retrieve last three 16bit indices
    {
        indices.x = (four16BitIndices.x >> 16) & 0xffff;
        indices.y = four16BitIndices.y & 0xffff;
        indices.z = (four16BitIndices.y >> 16) & 0xffff;
    }

    return indices;
}

inline void GenerateCameraRay(DXR_Scene_CB sceneCB, uint2 index, out float3 origin, out float3 direction)
{
    float2 xy = index + 0.5f; // center in the middle of the pixel.
    float2 screenPos = xy / DispatchRaysDimensions().xy * 2.0 - 1.0;

    // Invert Y for DirectX-style coordinates.
    screenPos.y = -screenPos.y;

    // Unproject the pixel coordinate into a ray
    float4 world = mul(float4(screenPos, 0, 1), sceneCB.viewProjectionInv);
    world.xyz /= world.w;
    origin = sceneCB.cameraPosition.xyz;
    direction = normalize(world.xyz - origin); // 카메라에서 픽셀(월드) 방향
}

// Generate a ray in world space for a camera pixel corresponding to an index from the dispatched 2D grid.
inline MyRay GenerateCameraRay(int2 index, in float3 cameraPosition, in float4x4 viewProjectionInv)
{
    float2 xy = index + 0.5f; // center in the middle of the pixel.
    float2 screenPos = xy / DispatchRaysDimensions().xy * 2.0 - 1.0;

	// Invert Y for DirectX-style coordinates.
    screenPos.y = -screenPos.y;

	// Unproject the pixel coordinate into a world positon.
    float4 world = mul(float4(screenPos, 0, 1), viewProjectionInv);
    world.xyz /= world.w;

    MyRay ray;
    ray.origin = cameraPosition;
    ray.direction = normalize(world.xyz - ray.origin);

    return ray;
}

// 무게중심 가중치 적용
float4 Apply_Barycentric_Float4(float4 vertexAttribute[3], BuiltInTriangleIntersectionAttributes attr)
{
    return vertexAttribute[0] +
        attr.barycentrics.x * (vertexAttribute[1] - vertexAttribute[0]) +
        attr.barycentrics.y * (vertexAttribute[2] - vertexAttribute[0]);
}
float3 Apply_Barycentric_Float3(float3 vertexAttribute[3], BuiltInTriangleIntersectionAttributes attr)
{
    return vertexAttribute[0] +
        attr.barycentrics.x * (vertexAttribute[1] - vertexAttribute[0]) +
        attr.barycentrics.y * (vertexAttribute[2] - vertexAttribute[0]);
}

float4 CalculateDiffuseLighting(DXR_Scene_CB sceneCB, float3 hitWorldPos, float3 normal)
{
    float3 pixelToLight = normalize(sceneCB.lightPosition.xyz - hitWorldPos);

    float fNDotL = max(0.f, dot(pixelToLight, normal)); // 람버트

    return sceneCB.lightDiffuseColor * fNDotL; // TODO : LocalRootSig의 오브젝트 알베도 적용
}

// Fresnel reflectance - schlick approximation.
float3 FresnelReflectanceSchlick(in float3 I, in float3 N, in float3 f0)
{
    float cosi = saturate(dot(-I, N));
    return f0 + (1 - f0) * pow(1 - cosi, 5);
}

#endif
