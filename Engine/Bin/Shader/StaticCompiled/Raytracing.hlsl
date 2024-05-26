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
    uint instanceIndex = InstanceIndex();
    OBJECT_CB_STATIC cur_object_cb_static = l_ObjectCB_Static.object_cb_static[instanceIndex];
#pragma region 충돌한 삼각형 추적하여 버텍스 정보 가져오기
    uint indexSizeInBytes = 2; // UINT16
    uint iTriangleIndexStrideInBytes = indexSizeInBytes * 3;
    uint iBaseOffsetInBytes = (cur_object_cb_static.startIndex_in_IB_SRV / 3 + PrimitiveIndex())
     * iTriangleIndexStrideInBytes; // 삼각형의 첫 인덱스 원소의 offset (바이트 단위)
        
    uint startIndex_in_VB_SRV = cur_object_cb_static.startIndex_in_VB_SRV;

    const uint3 indices3 = Load3x16BitIndices(Indices, iBaseOffsetInBytes);
    
    float4 triangleColorArr[3] =
    {
        Vertices[startIndex_in_VB_SRV + indices3[0]].color,
        Vertices[startIndex_in_VB_SRV + indices3[1]].color,
        Vertices[startIndex_in_VB_SRV + indices3[2]].color
    };
    float4 barycentricColor = Apply_Barycentric_Float4(triangleColorArr, attr);
    float3 vertexNormalArr[3] =
    {
        Vertices[startIndex_in_VB_SRV + indices3[0]].normal,
		Vertices[startIndex_in_VB_SRV + indices3[1]].normal,
		Vertices[startIndex_in_VB_SRV + indices3[2]].normal
    };
    float3 barycentricNormal = Apply_Barycentric_Float3(vertexNormalArr, attr);
    float3 hitWorldPos = WorldRayOrigin() + RayTCurrent() * WorldRayDirection();

    float3 triangleNormal = barycentricNormal;

    triangleNormal = mul(triangleNormal, l_ObjectCB_Dynamic.object_cb_dynamic[instanceIndex].InvTranspose).xyz;
    triangleNormal = normalize(triangleNormal);
#pragma endregion 충돌한 삼각형 추적하여 버텍스 정보 가져오기

    float3 hitWorldPosition = HitWorldPosition();
    MyRay shadowRay = { hitWorldPosition, normalize(g_sceneCB.lightPosition.xyz - hitWorldPosition) };
    bool shadowRayHit = TraceShadowRayAndReportIfHit(Scene_TLAS, shadowRay, payload.recursionDepth);
        

    float4 reflectedColor = float4(0, 0, 0, 0);
    if (cur_object_cb_static.reflectanceCoef > 0.001f)
    {
		// Trace a reflection ray.
        MyRay reflectionRay = { hitWorldPosition, reflect(WorldRayDirection(), triangleNormal) };
        float4 reflectionColor = TraceRadianceRay(Scene_TLAS, reflectionRay, payload.recursionDepth);

        // 금속같은 물체 반사
        float3 fresnelR = FresnelReflectanceSchlick(WorldRayDirection(), triangleNormal, barycentricColor);
        reflectedColor = cur_object_cb_static.reflectanceCoef * float4(fresnelR, 1) * reflectionColor;
    }


    float4 phongColor = CalculatePhongLighting(g_sceneCB, hitWorldPosition, barycentricColor, triangleNormal, shadowRayHit);
    float4 diffuseColor = CalculateDiffuseLighting(g_sceneCB, cur_object_cb_static.albedo, hitWorldPos, triangleNormal);
    float4 lightColor = diffuseColor + g_sceneCB.lightAmbientColor;
    //payload.color = l_ObjectCB_Static.object_cb_static[InstanceIndex()].albedo; // for debug albedo


    payload.color = Apply_Barycentric_Float4(triangleColorArr, attr) * lightColor;
    payload.color = Apply_Barycentric_Float4(triangleColorArr, attr);
    payload.color = Apply_Barycentric_Float4(triangleColorArr, attr) * phongColor;
    payload.color += reflectedColor;

   


}

[shader("miss")]
void MissShader_Default(inout MyRayPayload payload)
{
    float4 background = float4(0.8f, 1.f, 1.f, 1.000000000f);
    payload.color = background;

}
[shader("miss")]
void MissShader_ShadowRay(inout ShadowRayPayload rayPayload)
{
    rayPayload.hit = false;

}
#endif // RAYTRACING_HLSL