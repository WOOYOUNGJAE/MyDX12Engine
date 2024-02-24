#pragma once

#include "MeshGeometry.h"

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
public:
    void Load_FromFile();

private:

};

