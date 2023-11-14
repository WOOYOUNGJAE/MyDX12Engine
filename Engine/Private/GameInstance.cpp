#include "GameInstance.h"
#include "Graphic_Device.h"
#include "CubeMesh.h"
IMPLEMENT_SINGLETON(CGameInstance)

CGameInstance::CGameInstance() : m_pGraphic_Device{ CGraphic_Device::Get_Instance() }
{
}

HRESULT CGameInstance::Free()
{
	return S_OK;
}

HRESULT CGameInstance::Init_Engine(const GRAPHIC_DESC& GraphicDesc, _Inout_ ID3D12Device** ppDevice)
{
	m_pGraphic_Device->Init_Graphic_Device(GraphicDesc.hWnd, GraphicDesc.eWinMode, GraphicDesc.iSizeX, GraphicDesc.iSizeY, ppDevice);

	CCubeMesh* pInstance = CCubeMesh::Create(); // TODO Temp, Cube Test
	return S_OK;
}

HRESULT CGameInstance::Release_Engine()
{
	Safe_Release(m_pGraphic_Device);
	return S_OK;
}
