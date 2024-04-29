#ifndef RAYTRACING_HLSL
#define RAYTRACING_HLSL

#define HLSL
#include "RaytracingHlslCompat.h"

RaytracingAccelerationStructure Scene_TLAS : register(t0, space1);
RWTexture2D<float4> gRenderTarget : register(u0, space1);
typedef BuiltInTriangleIntersectionAttributes MyAttributes;
struct RayPayload
{
    float4 color;
};


[shader("raygeneration")]
void MyRaygenShader()
{
    float2 lerpValues = (float2) DispatchRaysIndex() / (float2) DispatchRaysDimensions();

    // Orthographic projection since we're raytracing in screen space.
    float3 rayDir = float3(0, 0, 1);
    float3 origin = float3(
        lerp(-1, 1, lerpValues.x),
        lerp(-1, 1, lerpValues.y),
        0.0f);

    // Trace the ray.
    // Set the ray's extents.
    RayDesc ray;
    ray.Origin = origin;
    ray.Direction = float3(0, 0, 1);

    // Set TMin to a non-zero small value to avoid aliasing issues due to floating - point errors.
    // TMin should be kept small to prevent missing geometry at close contact areas.
    ray.TMin = 0.001;
    ray.TMax = 10000.0;
    RayPayload payload = { float4(0, 0, 0, 0) };
    TraceRay(Scene_TLAS, 0, ~0, 0, 1, 0, ray, payload);

    // Write the raytraced color to the output texture.
    gRenderTarget[DispatchRaysIndex().xy] = payload.color;
    //gRenderTarget[DispatchRaysIndex().xy] = float4(1,1,1,1);
}

[shader("closesthit")]
void MyClosestHitShader(inout RayPayload payload, in MyAttributes attr)
{
    float4 color = float4(0, 0, 0, 0);

    payload.color = color;
}

[shader("miss")]
void MyMissShader(inout RayPayload payload)
{
    float4 background = float4(0.0f, 0.2f, 0.4f, 1.0f);
    //background = float4(0.662745118f, 0.662745118f, 0.662745118f, 1.000000000f);
    payload.color = background;
}

#endif // RAYTRACING_HLSL