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
#include "DXRRenderer.h"
#include "DXRResource.h"
#include "SDSManager.h"
#include "BVH.h"
#include "SceneNode_AABB.h"
#pragma endregion
IMPLEMENT_SINGLETON(CGameInstance)

CGameInstance::CGameInstance() :
	m_pDeviceResource(CDeviceResource::Get_Instance()),
	m_pComponentManager(CComponentManager::Get_Instance()),
	m_pGameObjectManager(CGameObjectManager::Get_Instance()),
	m_pPipelineManager(CPipelineManager::Get_Instance()),
	m_pAssetManager(CAssetManager::Get_Instance()),
	m_pD3DResourceManager(CD3DResourceManager::Get_Instance()),
	m_pInputManager(CInputManager::Get_Instance()),
	m_pCameraManager(CCameraManager::Get_Instance()),
	m_pSDSManager(CSDSManager::Get_Instance()),
	m_pLoadHelper(CLoadHelper::Get_Instance())
#if DXR_ON
	,m_pDxrResource(CDXRResource::Get_Instance())
#endif
{
	Safe_AddRef(m_pDeviceResource);
	Safe_AddRef(m_pComponentManager);
	Safe_AddRef(m_pGameObjectManager);
	Safe_AddRef(m_pPipelineManager);
	Safe_AddRef(m_pAssetManager);
	Safe_AddRef(m_pD3DResourceManager);
	Safe_AddRef(m_pInputManager);
	Safe_AddRef(m_pCameraManager);
	Safe_AddRef(m_pSDSManager);
#if DXR_ON
	Safe_AddRef(m_pDxrResource);
#endif
}

HRESULT CGameInstance::Free()
{
#if DXR_ON
	Safe_Release(m_pDxrResource);
	Safe_Release(m_pDxrRenderer);
#endif
	Safe_Release(m_pLoadHelper);
	Safe_Release(m_pSDSManager);
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
	HRESULT hr = S_OK;
	hr = m_pDeviceResource->Init_Graphic_Device(GraphicDesc.hWnd, GraphicDesc.eWinMode, GraphicDesc.iSizeX, GraphicDesc.iSizeY, ppDevice);
	if (FAILED(hr)) { return E_FAIL; }
	m_pHwndClient = &GraphicDesc.hWnd;

#ifdef DXR_ON
	hr = Init_DXR();
	if (FAILED(hr)) { return E_FAIL; }
#endif

	hr = m_pComponentManager->Initialize();
	if (FAILED(hr)) { return E_FAIL; }
	hr = m_pGameObjectManager->Initialize();
	if (FAILED(hr)) { return E_FAIL; }

	hr = m_pPipelineManager->Initialize();
	if (FAILED(hr)) { return E_FAIL; }

	m_pD3DResourceManager->Initialize();

	hr = m_pLoadHelper->Initialize();;
	if (FAILED(hr)) { return E_FAIL; }


	//hr = Init_DXR();

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
#if DXR_ON
	CDXRResource::Destroy_Instance();
#endif
	CLoadHelper::Destroy_Instance();
	CSDSManager::Destroy_Instance();
	CCameraManager::Destroy_Instance();
	CInputManager::Destroy_Instance();
	CD3DResourceManager::Destroy_Instance();
	CPipelineManager::Destroy_Instance();
	CAssetManager::Destroy_Instance();
	CComponentManager::Destroy_Instance();
	CGameObjectManager::Destroy_Instance();
	CDeviceResource::Destroy_Instance();
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

#if DXR_ON
void CGameInstance::Build_AccelerationStructureTree(CGameObject** pGameObjArr, UINT iArrSize)
{
	CBVH* pAccelerationTreeInstance = CBVH::Create(); // Manager에서 삭제

	CSceneNode** pChildNodeArr = new CSceneNode*[iArrSize];
	for (UINT i = 0; i < iArrSize; ++i)
	{
		CSceneNode* pNodeInstance = m_pSDSManager->FindandGet_LeafNode(pGameObjArr[i], SDS_AS);
		if (pNodeInstance == nullptr)
		{
			MSG_BOX("Build AS Tree Failed : Child Node Doesn't Exist or Didn't Registered to Manager");
			return;
		}
		pChildNodeArr[i] = pNodeInstance;
	}

	m_pDxrResource->Reset_CommandList();

	CSceneNode_AABB* pNodeTLAS = CSceneNode_AABB::Create(pChildNodeArr, iArrSize, true);
	pAccelerationTreeInstance->Set_Root(pNodeTLAS);

	m_pSDSManager->Push_AccelerationTree(pAccelerationTreeInstance);

	m_pDxrResource->Close_CommandList();
	m_pDxrResource->Execute_CommnadList();

	Safe_Delete_Array(pChildNodeArr);
}
#endif

CRenderer* CGameInstance::Get_Renderer()
{
	return m_pComponentManager->Get_Instance()->Get_Renderer();
}

#if DXR_ON
HRESULT CGameInstance::Init_DXR()
{
	HRESULT hr = S_OK;

	hr = m_pDxrResource->Initialize();
	if (FAILED(hr)) { return E_FAIL; }

	m_pDxrRenderer = CDXRRenderer::Create();
	if (m_pDxrRenderer == nullptr) { hr = E_FAIL; }

	return hr;
}
#endif