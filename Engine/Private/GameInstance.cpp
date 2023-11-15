#include "GameInstance.h"
#include "Graphic_Device.h"
#include "ComponentManager.h"
#include "Component.h"
#include "CubeMesh.h"
IMPLEMENT_SINGLETON(CGameInstance)

CGameInstance::CGameInstance() :
m_pGraphic_Device(CGraphic_Device::Get_Instance()),
m_pComponentManager(CComponentManager::Get_Instance())
{
	Safe_AddRef(m_pGraphic_Device);
	Safe_AddRef(m_pComponentManager);
}

HRESULT CGameInstance::Free()
{
	Safe_Release(m_pComponentManager);
	Safe_Release(m_pGraphic_Device);

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

	return S_OK;
}

HRESULT CGameInstance::Add_ComPrototype(const wstring& strTag, CComponent* pComInstance)
{
	if (m_pComponentManager == nullptr ||
		FAILED(m_pComponentManager->Add_Prototype(strTag, pComInstance)))
	{
		MSG_BOX("GameInstance: Failed To Add Component Prototype");
		return E_FAIL;
	}
	;
	return S_OK;
}

CComponent* CGameInstance::Clone_ComPrototye(const wstring& strTag, void* pArg)
{
	CComponent* pInstnace = m_pComponentManager->Clone_Component(strTag, pArg);

	if (pInstnace == nullptr)
	{
		MSG_BOX("GameInstance: Failed To Clone Component Prototype");
		return nullptr;
	}

	return pInstnace;
}
