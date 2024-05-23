#ifndef RAYTRACING_HLSL
#define RAYTRACING_HLSL

#define HLSL
#include "RaytracingFunctions.hlsli"

// Global RootSig
RaytracingAccelerationStructure Scene_TLAS : register(t0, space1);
RWTexture2D<float4> gRenderTarget : register(u0, space1);
ByteAddressBuffer Indices : register(t1, space1);
StructuredBuffer<VertexPositionNormalColorTexture> Vertices : register(t2, space1);
ConstantBuffer<DXR_Scene_CB> g_sceneCB : register(b0, space1);

// Local RootSig
ConstantBuffer<OBJECT_CB_STATIC_Arr> l_ObjectCB_Static : register(b1, space1);
ConstantBuffer<OBJECT_CB_DYNAMIC_Arr> l_ObjectCB_Dynamic : register(b2, space1);

// Trace a radiance ray into the scene and returns a shaded color.
float4 TraceRadianceRay(RaytracingAccelerationStructure scene, in MyRay ray, in UINT currentRayRecursionDepth)
{
    if (currentRayRecursionDepth >= MAX_RAY_RECURSION_DEPTH)
    {
        return float4(0, 0, 0, 0);
    }
//----------------------
	// Set the ray's extents.
    RayDesc rayDesc;
    rayDesc.Origin = ray.origin;
	rayDesc.Direction = ray.direction;
	// Set TMin to a zero value to avoid aliasing artifacts along contact areas.
	// Note: make sure to enable face culling so as to avoid surface face fighting.
    rayDesc.TMin = 0.001;
    rayDesc.TMax = 10000;
    MyRayPayload rayPayload = { float4(0, 0, 0, 0), currentRayRecursionDepth + 1 };

    TraceRay(scene,
    RAY_FLAG_CULL_BACK_FACING_TRIANGLES,
    //0,
    TraceRayParameters::InstanceMask,
    TraceRayParameters::HitGroup::Offset[RayType::Radiance],
    TraceRayParameters::HitGroup::GeometryStride,
    TraceRayParameters::MissShader::Offset[RayType::Radiance],
    rayDesc,
    rayPayload);

    return rayPayload.color;
}

[shader("raygeneration")]
void MyRaygenShader()
{
    //    float3 rayDir;
    //    float3 origin;
    
    //// Generate a ray for a camera pixel corresponding to an index from the dispatched 2D grid.
    //    GenerateCameraRay(g_sceneCB, DispatchRaysIndex().xy, origin, rayDir);

    //// Trace the ray.
    //// Set the ray's extents.
    //    RayDesc rayDesc;
    //    rayDesc.Origin = origin;
    //    rayDesc.Direction = rayDir;
    //// Set TMin to a non-zero small value to avoid aliasing issues due to floating - point errors.
    //// TMin should be kept small to prevent missing geometry at close contact areas.
    //    rayDesc.TMin = 0.001;
    //    rayDesc.TMax = 10000.0;
    //    MyRayPayload payload = { float4(0, 0, 0, 0), 2 };
    ////TraceRay(Scene_TLAS, RAY_FLAG_CULL_BACK_FACING_TRIANGLES, ~0, 0, 1, 0, ray, payload);
    //    TraceRay(Scene_TLAS, 0, ~0, 0, 1, 0, rayDesc, payload);

    ////// Cast a ray into the scene and retrieve a shaded color.
    ////UINT currentRecursionDepth = 0;
    ////float4 color = TraceRadianceRay(Scene_TLAS, ray, currentRecursionDepth);

    //// Write the raytraced color to the output texture.
    //    gRenderTarget[DispatchRaysIndex().xy] = payload.color;


    //------------------------
	// Generate a ray for a camera pixel corresponding to an index from the dispatched 2D grid.
    MyRay ray = GenerateCameraRay(DispatchRaysIndex().xy, g_sceneCB.cameraPosition.xyz, g_sceneCB.viewProjectionInv);
 
    // Cast a ray into the scene and retrieve a shaded color.
    UINT currentRecursionDepth = 0;
    float4 color = TraceRadianceRay(Scene_TLAS, ray, currentRecursionDepth);

    // Write the raytraced color to the output texture.
    gRenderTarget[DispatchRaysIndex().xy] = color;
}

float3 HitWorldPosition()
{
    return WorldRayOrigin() + RayTCurrent() * WorldRayDirection();
}

[shader("closesthit")]
void MyClosestHitShader(inout MyRayPayload payload, in MyAttributes attr)
{
    uint indexSizeInBytes = 2; // UINT16
    uint iTriangleIndexStrideInBytes = indexSizeInBytes * 3;
    uint iBaseOffsetInBytes = (l_ObjectCB_Static.object_cb_static[InstanceIndex()].startIndex_in_IB_SRV / 3 + PrimitiveIndex())
     * iTriangleIndexStrideInBytes; // 삼각형의 첫 인덱스 원소의 offset (바이트 단위)
    
    uint startIndex_in_VB_SRV = l_ObjectCB_Static.object_cb_static[InstanceIndex()].startIndex_in_VB_SRV; //0 or 24
    const uint3 indices3 = Load3x16BitIndices(Indices, iBaseOffsetInBytes);

    float4 triangleColorArr[3] =
    {
        Vertices[startIndex_in_VB_SRV + indices3[0]].color,
        Vertices[startIndex_in_VB_SRV + indices3[1]].color,
        Vertices[startIndex_in_VB_SRV + indices3[2]].color
    };

    float3 vertexNormalArr[3] =
    {
        Vertices[startIndex_in_VB_SRV + indices3[0]].normal,
		Vertices[startIndex_in_VB_SRV + indices3[1]].normal,
		Vertices[startIndex_in_VB_SRV + indices3[2]].normal
    };

    float3 hitWorldPos = WorldRayOrigin() + RayTCurrent() * WorldRayDirection();

    float3 triangleNormal = Apply_Barycentric_Float3(vertexNormalArr, attr);

    triangleNormal = mul(triangleNormal, l_ObjectCB_Dynamic.object_cb_dynamic[InstanceIndex()].InvTranspose).xyz;
    triangleNormal = normalize(triangleNormal);

        

    float4 reflectedColor = float4(0, 0, 0, 0);
    if (true)
    {
   // Trace a reflection ray.
        MyRay reflectionRay = { HitWorldPosition(), reflect(WorldRayDirection(), triangleNormal) };
        float4 reflectionColor = TraceRadianceRay(Scene_TLAS, reflectionRay, payload.recursionDepth);

        float3 fresnelR = FresnelReflectanceSchlick(WorldRayDirection(), triangleNormal, float3(0.9f, 0.9f, 0.9f) /*albedo*/);
        reflectedColor = 0.25f /*reflectanceCoef*/ * float4(fresnelR, 1) * reflectionColor;
    }


    float4 diffuseColor = CalculateDiffuseLighting(g_sceneCB, hitWorldPos, triangleNormal);
    float4 lightColor = diffuseColor + g_sceneCB.lightAmbientColor;
    //payload.color = Apply_Barycentric_Float4(triangleColorArr, attr) * lightColor;
    payload.color = l_ObjectCB_Static.object_cb_static[InstanceIndex()].albedo; // for debug albedo
    payload.color = Apply_Barycentric_Float4(triangleColorArr, attr);

   

        payload.color += reflectedColor;

}

[shader("miss")]
void MyMissShader(inout MyRayPayload payload)
{
    float4 background = float4(0.8f, 1.f, 1.f, 1.000000000f);
    payload.color = background;

}

#endif // RAYTRACING_HLSL