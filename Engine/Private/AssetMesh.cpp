#include "AssetMesh.h"

CAssetMesh::CAssetMesh(CAssetMesh& rhs) : CMeshData(rhs),
m_vecVertexData(rhs.m_vecVertexData),
m_vecIndexData(rhs.m_vecIndexData)
{
}

CAssetMesh* CAssetMesh::Create()
{
	return new CAssetMesh();
}

CMeshData* CAssetMesh::Clone(void* pArg)
{
	CAssetMesh* pInstance = new CAssetMesh(*this);

	pInstance->Initialize(pArg);

	return pInstance;
}

HRESULT CAssetMesh::Initialize(void* pArg)
{
	return S_OK;
}

HRESULT CAssetMesh::Free()
{
	return CMeshData::Free();
}

void CAssetMesh::Load_FromFile()
{

}
