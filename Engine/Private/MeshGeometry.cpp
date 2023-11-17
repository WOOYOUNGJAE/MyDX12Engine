#include "MeshGeometry.h"
#include "Graphic_Device.h"
#include "UploadBuffer.h"

CMeshGeometry::CMeshGeometry() :
	m_pDevice(CGraphic_Device::Get_Instance()->Get_Device()),
	m_pCommandList(CGraphic_Device::Get_Instance()->Get_CommandList())
{
}

HRESULT CMeshGeometry::Initialize_Prototype()
{
	if (m_pDevice == nullptr || m_pCommandList == nullptr)
	{
		MSG_BOX("MeshGeometry: Device Null");
		return E_FAIL;
	}

	return S_OK;
}

HRESULT CMeshGeometry::Free()
{
	/*Safe_Delete_Array(m_vertexData);
	Safe_Delete_Array(m_vertexPosArr);*/

	if (FAILED(CComponent::Free()))
	{
		return E_FAIL;
	}

	return S_OK;
}
