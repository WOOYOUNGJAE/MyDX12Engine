float4x4 Inverse(float4x4 m)
{
    float3 a = m[0].xyz;
    float3 b = m[1].xyz;
    float3 c = m[2].xyz;
    float3 d = m[3].xyz;

    float3 cross_ab_cd = cross(a, b);
    float3 cross_bc_da = cross(b, c);
    float3 cross_cd_ab = cross(c, d);

    float det = dot(a, cross_bc_da);

    float4x4 inverseMatrix;

    if (det != 0.0)
    {
        inverseMatrix[0] = float4(cross_bc_da / det, 0.0);
        inverseMatrix[1] = float4(cross_cd_ab / det, 0.0);
        inverseMatrix[2] = float4(cross(b, a) / det, 0.0);
        inverseMatrix[3] = float4(cross(d, a) / det, 0.0);

        inverseMatrix = transpose(inverseMatrix);
    }

    return inverseMatrix;
}