#pragma once

#include "MeshGeometry.h"
NAMESPACE_(Engine)
using std::vector;
class CAssetMesh :
    public CMeshData
{
protected:
    CAssetMesh() = default;
    CAssetMesh(CAssetMesh& rhs) : CMeshData(rhs) {}
    ~CAssetMesh() override = default;

public:
    static CAssetMesh* Create();
    CMeshData* Clone(void* pArg) override;
    HRESULT Initialize(void* pArg) override;
    HRESULT Free() override;
public: // getter
    vector<VertexPositionNormalTexture*>& Get_vecVertices() { return m_vecVertexData; }
public:
    void Load_FromFile();

private:
    vector<VertexPositionNormalTexture*> m_vecVertexData;

};

_NAMESPACE