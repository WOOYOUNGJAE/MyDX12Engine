#include "MeshGeometry.h"

HRESULT CMeshGeometry::Free()
{
	Safe_Delete_Array(m_vertexData);
	Safe_Delete_Array(m_vertexPosArr);

	if (FAILED(CComponent::Free()))
	{
		return E_FAIL;
	}

	return S_OK;
}
