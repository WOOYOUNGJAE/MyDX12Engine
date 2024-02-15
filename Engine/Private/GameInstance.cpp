#pragma region Includes
#include "GameInstance.h"
#include "Graphic_Device.h"
#include "ComponentManager.h"
#include "GameObjectManager.h"
#include "Component.h"
#include "CubeMesh.h"
#include "PipelineManager.h"
#include "AssetManager.h"
#include "LoadHelper.h"
#include "D3DResourceManager.h"
#pragma endregion
IMPLEMENT_SINGLETON(CGameInstance)

CGameInstance::CGameInstance() :
m_pGraphic_Device(CGraphic_Device::Get_Instance()),
m_pComponentManager(CComponentManager::Get_Instance()),
m_pGameObjectManager(CGameObjectManager::Get_Instance()),
m_pPipelineManager(CPipelineManager::Get_Instance()),
m_pAssetManager(CAssetManager::Get_Instance()),
m_pD3DResourceManager(CD3DResourceManager::Get_Instance()),
m_pLoadHelper(CLoadHelper::Get_Instance())
{
	Safe_AddRef(m_pGraphic_Device);
	Safe_AddRef(m_pComponentManager);
	Safe_AddRef(m_pGameObjectManager);
	Safe_AddRef(m_pPipelineManager);
	Safe_AddRef(m_pD3DResourceManager);
	Safe_AddRef(m_pAssetManager);
}

HRESULT CGameInstance::Free()
{
	Safe_Release(m_pLoadHelper);
	Safe_Release(m_pD3DResourceManager);
	Safe_Release(m_pAssetManager);
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
	m_pD3DResourceManager->Initialize();
	m_pLoadHelper->Initialize();

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

void CGameInstance::Render_Tick()
{
	m_pGameObjectManager->Render_Tick();
}

void CGameInstance::Engine_Tick(FLOAT fDeltaTime)
{
	Tick(fDeltaTime);
	Late_Tick(fDeltaTime);
	Render_Tick();
}

void CGameInstance::Release_Engine()
{
	// Destroy Managers or Singletons, 최종 삭제
	CAssetManager::Destroy_Instance();
	CD3DResourceManager::Destroy_Instance();
	CPipelineManager::Destroy_Instance();
	CComponentManager::Destroy_Instance();
	CGameObjectManager::Destroy_Instance();
	CGraphic_Device::Destroy_Instance();
	CLoadHelper::Destroy_Instance();
	CGameInstance::Get_Instance()->Destroy_Instance();
}

UINT CGameInstance::Get_CbvSrvUavDescriptorSize() const
{
	return m_pGraphic_Device->Get_CbvSrvUavDescriptorSize();
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

CComponent* CGameInstance::Clone_ComPrototype(const wstring& strTag, void* pArg)
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

HRESULT CGameInstance::Add_GameObject_InScene(const wstring& strPrototypeTag, UINT eLayerEnum, void* pArg)
{
	return m_pGameObjectManager->Add_GameObject_InScene(strPrototypeTag, eLayerEnum, pArg);
}

HRESULT CGameInstance::Add_GameObject_InScene(const wstring& strPrototypeTag, UINT eLayerEnum,
                                              CGameObject** pOutObj, void* pArg)
{
	HRESULT hr = m_pGameObjectManager->Add_GameObject_InScene(strPrototypeTag, eLayerEnum, pOutObj, pArg);
	return hr;
}

CGameObject* CGameInstance::FindandGet_GameObj_Cloned(const wstring& strPrototypeTag, UINT eLayerEnum, UINT iClonedNum)
{
	return m_pGameObjectManager->FindandGet_Cloned(strPrototypeTag, eLayerEnum, iClonedNum);
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

CRenderer* CGameInstance::Get_Renderer()
{
	return m_pComponentManager->Get_Instance()->Get_Renderer();
}
