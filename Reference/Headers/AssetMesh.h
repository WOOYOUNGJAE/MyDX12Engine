#pragma once

#include "MeshData.h"
NAMESPACE_(Engine)
using std::vector;
using std::wstring;
class CAssetMesh :
    public CMeshData
{
protected:
    CAssetMesh() = default;
    CAssetMesh(CAssetMesh& rhs);
    ~CAssetMesh() override = default;

public:
    static CAssetMesh* Create();
    CMeshData* Clone(void* pArg = nullptr) override;
    HRESULT Initialize_Prototype() override;
    HRESULT ReInit_Prototype(); // Init_Prototype와 달리 수동으로 호출
    HRESULT Initialize(void* pArg) override;
    HRESULT Free() override;
public: // getter
    vector<VertexPositionNormalTexture>& Get_vecVertices() { return m_vecVertexData; }
    vector<UINT32>& Get_vecIndices() { return m_vecIndexData; }
    wstring& Get_Path() { return m_strPath; }
public:
    void Load_FromFile();

private:
    vector<VertexPositionNormalTexture> m_vecVertexData;
    vector<UINT32> m_vecIndexData;
    wstring m_strPath;
};

_NAMESPACE