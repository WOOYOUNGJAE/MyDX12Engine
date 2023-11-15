#include "GameInstance.h"
#include "Graphic_Device.h"
#include "ComponentManager.h"
#include "GameObjectManager.h"
#include "GameObject.h"
#include "Component.h"
#include "CubeMesh.h"
IMPLEMENT_SINGLETON(CGameInstance)

CGameInstance::CGameInstance() :
m_pGraphic_Device(CGraphic_Device::Get_Instance()),
m_pComponentManager(CComponentManager::Get_Instance()),
m_pGameObjectManager(CGameObjectManager::Get_Instance())
{
	Safe_AddRef(m_pGraphic_Device);
	Safe_AddRef(m_pComponentManager);
	Safe_AddRef(m_pGameObjectManager);
}

HRESULT CGameInstance::Free()
{
	Safe_Release(m_pGameObjectManager);
	Safe_Release(m_pComponentManager);
	Safe_Release(m_pGraphic_Device);

	return S_OK;
}

HRESULT CGameInstance::Init_Engine(const GRAPHIC_DESC& GraphicDesc, _Inout_ ID3D12Device** ppDevice)
{
	m_pGraphic_Device->Init_Graphic_Device(GraphicDesc.hWnd, GraphicDesc.eWinMode, GraphicDesc.iSizeX, GraphicDesc.iSizeY, ppDevice);
	m_pComponentManager->Initialize();
	m_pGameObjectManager->Initialize();

	//CCubeMesh* pInstance = CCubeMesh::Create(); // TODO Temp, Cube Test
	return S_OK;
}

void CGameInstance::Tick(_float fDeltaTime)
{
	m_pGameObjectManager->Tick(fDeltaTime);
}

void CGameInstance::Late_Tick(_float fDeltaTime)
{
	m_pGameObjectManager->Late_Tick(fDeltaTime);
}

void CGameInstance::Release_Engine()
{
	// Destroy Managers or Singletons, ���� ����
	CGraphic_Device::Destroy_Instance();
	CComponentManager::Destroy_Instance();
	CGameObjectManager::Destroy_Instance();
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

HRESULT CGameInstance::Add_GameObjPrototype(const wstring& strTag, CGameObject* pInstance)
{
	return m_pGameObjectManager->Add_Prototype(strTag, pInstance);
}

CGameObject* CGameInstance::Clone_GameObject(const wstring& strPrototypeTag, void* pArg)
{
	return m_pGameObjectManager->Clone_GameObject(strPrototypeTag, pArg);
}

HRESULT CGameInstance::Add_GameObject_InScene(const wstring& strPrototypeTag, const wstring& strLayerTag, void* pArg)
{
	return m_pGameObjectManager->Add_GameObject_InScene(strPrototypeTag, strLayerTag, pArg);
}
