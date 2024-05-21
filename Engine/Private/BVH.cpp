#include "BVH.h"
#include "SceneNode_AABB.h"

CBVH* CBVH::Create()
{
	return new CBVH;
}

HRESULT CBVH::Free()
{
	HRESULT hr = S_OK;

	Safe_Release(m_pRoot);

	return hr;
}

void CBVH::Rebuild_Root_TLAS()
{
	m_pRoot->ReBuild_TLAS();
}
