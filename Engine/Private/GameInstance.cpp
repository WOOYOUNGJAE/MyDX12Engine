#pragma region Includes
#include "GameInstance.h"
#include "DeviceResource.h"
#include "ComponentManager.h"
#include "GameObjectManager.h"
#include "Component.h"
#include "CubeMesh.h"
#include "PipelineManager.h"
#include "AssetManager.h"
#include "InputManager.h"
#include "LoadHelper.h"
#include "D3DResourceManager.h"
#include "CameraManager.h"
#pragma endregion
IMPLEMENT_SINGLETON(CGameInstance)

CGameInstance::CGameInstance() :
#pragma region Allocate Managers
	m_pDeviceResource(CDeviceResource::Get_Instance()),
	m_pComponentManager(CComponentManager::Get_Instance()),
	m_pGameObjectManager(CGameObjectManager::Get_Instance()),
	m_pPipelineManager(CPipelineManager::Get_Instance()),
	m_pAssetManager(CAssetManager::Get_Instance()),
	m_pD3DResourceManager(CD3DResourceManager::Get_Instance()),
	m_pInputManager(CInputManager::Get_Instance()),
	m_pCameraManager(CCameraManager::Get_Instance()),
	m_pLoadHelper(CLoadHelper::Get_Instance())
#pragma endregion
{
	Safe_AddRef(m_pDeviceResource);
	Safe_AddRef(m_pComponentManager);
	Safe_AddRef(m_pGameObjectManager);
	Safe_AddRef(m_pPipelineManager);
	Safe_AddRef(m_pD3DResourceManager);
	Safe_AddRef(m_pInputManager);
	Safe_AddRef(m_pCameraManager);
	Safe_AddRef(m_pAssetManager);
}

HRESULT CGameInstance::Free()
{
	Safe_Release(m_pLoadHelper);
	Safe_Release(m_pCameraManager);
	Safe_Release(m_pInputManager);
	Safe_Release(m_pD3DResourceManager);
	Safe_Release(m_pAssetManager);
	Safe_Release(m_pPipelineManager);
	Safe_Release(m_pGameObjectManager);
	Safe_Release(m_pComponentManager);
	Safe_Release(m_pDeviceResource);
	

	return S_OK;
}

HRESULT CGameInstance::Init_Engine(GRAPHIC_DESC& GraphicDesc, _Inout_ ID3D12Device** ppDevice)
{
	if (FAILED(m_pDeviceResource->Init_Graphic_Device(GraphicDesc.hWnd, GraphicDesc.eWinMode, GraphicDesc.iSizeX, GraphicDesc.iSizeY, ppDevice)))
	{
		return E_FAIL;
	}

	m_pHwndClient = &GraphicDesc.hWnd;

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
	m_pInputManager->Tick(m_pHwndClient);
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
	CCameraManager::Destroy_Instance();
	CInputManager::Destroy_Instance();
	CD3DResourceManager::Destroy_Instance();
	CPipelineManager::Destroy_Instance();
	CComponentManager::Destroy_Instance();
	CGameObjectManager::Destroy_Instance();
	CDeviceResource::Destroy_Instance();
	CLoadHelper::Destroy_Instance();
	CGameInstance::Get_Instance()->Destroy_Instance();
}

UINT CGameInstance::Get_CbvSrvUavDescriptorSize() const
{
	return m_pDeviceResource->Get_CbvSrvUavDescriptorSize();
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

void CGameInstance::Set_MainCam(wstring strName)
{
	m_pCameraManager->Set_MainCam(strName);
}

CRenderer* CGameInstance::Get_Renderer()
{
	return m_pComponentManager->Get_Instance()->Get_Renderer();
}
