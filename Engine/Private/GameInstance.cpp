#pragma region Includes
#include "GameInstance.h"
#include "Graphic_Device.h"
#include "ComponentManager.h"
#include "GameObjectManager.h"
#include "GameObject.h"
#include "Component.h"
#include "CubeMesh.h"
#include "PipelineManager.h"  
#pragma endregion

IMPLEMENT_SINGLETON(CGameInstance)

CGameInstance::CGameInstance() :
m_pGraphic_Device(CGraphic_Device::Get_Instance()),
m_pComponentManager(CComponentManager::Get_Instance()),
m_pGameObjectManager(CGameObjectManager::Get_Instance()),
m_pPipelineManager(CPipelineManager::Get_Instance())
{
	Safe_AddRef(m_pGraphic_Device);
	Safe_AddRef(m_pComponentManager);
	Safe_AddRef(m_pGameObjectManager);
	Safe_AddRef(m_pPipelineManager);
}

HRESULT CGameInstance::Free()
{
	Safe_Release(m_pPipelineManager);
	Safe_Release(m_pGameObjectManager);
	Safe_Release(m_pComponentManager);
	Safe_Release(m_pGraphic_Device);
	

	return S_OK;
}

HRESULT CGameInstance::Init_Engine(const GRAPHIC_DESC& GraphicDesc, _Inout_ ID3D12Device** ppDevice)
{
	if (FAILED(m_pGraphic_Device->Init_Graphic_Device(GraphicDesc.hWnd, GraphicDesc.eWinMode, GraphicDesc.iSizeX, GraphicDesc.iSizeY, ppDevice)))
	{
		return E_FAIL;
	}

	m_pComponentManager->Initialize();
	m_pGameObjectManager->Initialize();

	if (FAILED(m_pPipelineManager->Initialize()))
	{
		return E_FAIL;
	}
	
	return S_OK;
}

void CGameInstance::Tick(_float fDeltaTime)
{
	m_pGameObjectManager->Tick(fDeltaTime);
}

void CGameInstance::Late_Tick(_float fDeltaTime)
{
	m_pGameObjectManager->Late_Tick(fDeltaTime);
	m_pPipelineManager->Pipeline_Tick(fDeltaTime);
	m_pPipelineManager->Render();
}

void CGameInstance::Release_Engine()
{
	// Destroy Managers or Singletons, 최종 삭제
	CPipelineManager::Destroy_Instance();
	CComponentManager::Destroy_Instance();
	CGameObjectManager::Destroy_Instance();
	CGraphic_Device::Destroy_Instance();
	CGameInstance::Get_Instance()->Destroy_Instance();
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

HRESULT CGameInstance::Add_GameObject_InScene(const wstring& strPrototypeTag, const wstring& strLayerTag,
	CGameObject** pOutObj, void* pArg)
{
	HRESULT hr = m_pGameObjectManager->Add_GameObject_InScene(strPrototypeTag, strLayerTag, pOutObj, pArg);
	return hr;
}

void CGameInstance::Update_ObjPipelineLayer(CGameObject* pObject, _uint ePsoEnum)
{
	if (pObject == nullptr)
	{
		MSG_BOX("GameInstance : UpdatePipelineLayer Failed : null");
		return;
	}
	m_pPipelineManager->Update_ObjPipelineLayer(pObject, (ENUM_PSO)ePsoEnum);
}
