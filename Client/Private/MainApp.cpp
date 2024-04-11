#include "pch.h"
#include "MainApp.h"
#include "Renderer.h"
#include "Client_Defines.h"
#include "GameInstance.h"
#include "Camera_Free.h"
#include "ClientLoader.h"
#include "Skybox.h"
#include "TreeBillboard.h"
#include "ZeldaDemo.h"
#if DXR_ON
#include "DXRRenderer.h"
#endif

#if IMGUI_ON
#include "Client_Imgui.h"
#endif

CInputManager* g_Input = nullptr;

CMainApp::CMainApp() : m_pGameInstance{ CGameInstance::Get_Instance()}
{
	Safe_AddRef(m_pGameInstance);
}

CMainApp::~CMainApp()
{
}

HRESULT CMainApp::Initialize()
{
	GRAPHIC_DESC graphic_desc{};
	graphic_desc.hWnd = g_hwnd;
	graphic_desc.iSizeX = g_iWinSizeX;
	graphic_desc.iSizeY = g_iWinSizeY;
	graphic_desc.eWinMode = GRAPHIC_DESC::WINMODE_WIN;

	HRESULT hr = 
	m_pGameInstance->Init_Engine(graphic_desc, &m_pDevice);
	if (FAILED(hr)) { return hr; }

	g_Input = CInputManager::Get_Instance();

	// Loader
	CClientLoader* pLoader = CClientLoader::Create();
	pLoader->Load();
	Safe_Release(pLoader);

	/*hr = m_pGameInstance->Init_DXR();
	if (FAILED(hr)) { return hr; }*/

#pragma region InLevel
	CGameObject* pObjectControlling = nullptr;
	GAMEOBJECT_INIT_DESC objDesc{};
	objDesc.vStartPos = Vector3(0.f, 0.f, 0.f);
	objDesc.vStartScale = Vector3::One;


	hr = m_pGameInstance->Add_GameObjPrototype(L"Camera_Free", CCamera_Free::Create());
	if (FAILED(hr)) { return hr; }
	hr = m_pGameInstance->Add_GameObjPrototype(L"ZeldaDemo", CZeldaDemo::Create());
	if (FAILED(hr)) { return hr; }
	hr = m_pGameInstance->Add_GameObjPrototype(L"Skybox", CSkybox::Create());
	if (FAILED(hr)) { return hr; }
	hr = m_pGameInstance->Add_GameObjPrototype(L"TreeBillboard", CTreeBillboard::Create());
	if (FAILED(hr)) { return hr; }


	objDesc.strTag = L"Camera_Free";
	objDesc.vStartPos = Vector3(0.f, 1.f, -5.f);
	hr = m_pGameInstance->Add_GameObject_InScene(L"Camera_Free", OBJ_LAYER_DEFAULT,	&objDesc);
	if (FAILED(hr)) { return hr; }

	m_pGameInstance->Set_MainCam(objDesc.strTag);


#pragma region Static GameObject

#if DXR_ON
	vector<CGameObject*> gameObjArr_For_AccelerationTree_Static;
#endif
//	objDesc.strTag = L"Cube";
//	objDesc.vStartPos = Vector3::Zero;
//	hr = m_pGameInstance->Add_GameObject_InScene(L"Cube", OBJ_LAYER_0, &pObjectControlling, &objDesc);
//	if (FAILED(hr)) { return hr; }
//#if DXR_ON
//	gameObjArr_For_AccelerationTree_Static.emplace_back(pObjectControlling);
//	//CGameInstance::Get_Instance()->Build_AccelerationStructureTree(gameObjArr_For_AccelerationTree_Static.data(), gameObjArr_For_AccelerationTree_Static.size());
//#endif DXR_ON


	objDesc.strTag = L"Triangle";
	objDesc.vStartPos = Vector3::Zero;
	hr = m_pGameInstance->Add_GameObject_InScene(L"Triangle", OBJ_LAYER_0, &pObjectControlling, &objDesc);
	if (FAILED(hr)) { return hr; }
	m_pGameInstance->Add_ClonedObj_To_Array_For_ShaderTable(pObjectControlling);
#if DXR_ON
	gameObjArr_For_AccelerationTree_Static.emplace_back(pObjectControlling);
	CGameInstance::Get_Instance()->Build_AccelerationStructureTree(gameObjArr_For_AccelerationTree_Static.data(), gameObjArr_For_AccelerationTree_Static.size());
#endif DXR_ON


#pragma endregion Static GameObject 


	/*objDesc.strTag = L"ZeldaDemo";
	objDesc.vStartPos = Vector3(2.f, 0.f, -5.f);
	hr = m_pGameInstance->Add_GameObject_InScene(L"ZeldaDemo", OBJ_LAYER_0, &pObjectControlling, &objDesc);
	if (FAILED(hr)) { return hr; }*/

	/*objDesc.vStartPos = Vector3(1.f, 1.f, 0.f);
	hr = m_pGameInstance->Add_GameObject_InScene(L"Cube", OBJ_LAYER_0, &pObjectControlling, &objDesc);
	if (FAILED(hr)) { return hr; }


	objDesc.strTag = L"Grid_10X10";
	objDesc.vStartPos = Vector3(-2.f, 0.f, -5.f);
	objDesc.vStartScale = Vector3(2, 2, 2);
	hr = m_pGameInstance->Add_GameObject_InScene(objDesc.strTag, OBJ_LAYER_0, &pObjectControlling, &objDesc);
	if (FAILED(hr)) { return hr; }

	objDesc.strTag = L"Cylinder_20_05_05";
	objDesc.vStartPos = Vector3(2.3f, 0.f, 1.5f);
	hr = m_pGameInstance->Add_GameObject_InScene(objDesc.strTag, OBJ_LAYER_0, &pObjectControlling, &objDesc);
	if (FAILED(hr)) { return hr; }

	objDesc.strTag = L"Sphere_15X15";
	objDesc.vStartPos = Vector3(0, 0, -7);
	objDesc.vStartScale = Vector3::One * 0.9f;
	hr = m_pGameInstance->Add_GameObject_InScene(objDesc.strTag, OBJ_LAYER_0, &pObjectControlling, &objDesc);
	if (FAILED(hr)) { return hr; }*/

	/*objDesc.strTag = L"Skybox";
	objDesc.vStartPos = Vector3(0,3,0);
	objDesc.vStartScale = Vector3::One;
	hr = m_pGameInstance->Add_GameObject_InScene(objDesc.strTag, OBJ_LAYER_0, &pObjectControlling, &objDesc);
	if (FAILED(hr)) { return hr; }*/

	/*objDesc.strTag = L"TreeBillboard";
	objDesc.vStartPos = Vector3(0, 0, 2);
	objDesc.vStartScale = Vector3::One;
	hr = m_pGameInstance->Add_GameObject_InScene(objDesc.strTag, OBJ_LAYER_0, &pObjectControlling, &objDesc);
	if (FAILED(hr)) { return hr; }*/
#pragma endregion InLevel

	hr = m_pGameInstance->Build_FrameResource_After_Loading_GameScene_Finished(
		/*TODO Clone된(프로토타입제외) 모든 렌더링해야 하는 오브젝트 개수, Factory패턴으로 수집한 정보 대입 예정*/
		15);
	if (FAILED(hr)) { return hr; }


	// Build DXR Shader Table
#if DXR_ON

#endif
	m_pGameInstance->Clear_ClonedObjArray();

	// Renderer
	m_pRenderer = m_pGameInstance->Get_Instance()->Get_Renderer();
	Safe_AddRef(m_pRenderer);

	if (m_pRenderer == nullptr)
	{
		MSG_BOX("MainApp: Renderer is nullptr");
		return E_FAIL;
	}

#if IMGUI_ON
	m_pClient_Imgui = CClient_Imgui::Create(m_pDevice);
#endif
#if DXR_ON
	m_pDXRRenderer = m_pGameInstance->Get_DXRRenderer();
	Safe_AddRef(m_pDXRRenderer);
#endif


	return S_OK;
}
void CMainApp::Tick(_float fDeltaTime)
{
#if DXR_ON
	m_pDXRRenderer->BeginRender();
	//m_pRenderer->BeginRender();
#else
	m_pRenderer->BeginRender();
#endif


	m_pGameInstance->Engine_Tick(fDeltaTime);

	m_pDXRRenderer->MainRender();
	//m_pRenderer->MainRender();

#if IMGUI_ON
	m_pClient_Imgui->Imgui_Tick();
	m_pClient_Imgui->Imgui_MainRender();
	m_pClient_Imgui->IMgui_EndRender();
	m_pClient_Imgui->Imgui_Present();
#else
	//m_pRenderer->EndRender();
	//m_pRenderer->Present();
	m_pDXRRenderer->EndRender();
	m_pDXRRenderer->Present();
#endif

}


HRESULT CMainApp::Free()
{
#if IMGUI_ON
	Safe_Release(m_pClient_Imgui);
#endif
#if DXR_ON
	Safe_Release(m_pDXRRenderer);
#endif

	Safe_Release(m_pRenderer);
	Safe_Release(m_pGameInstance);

	CGameInstance::Release_Engine();

	
	return S_OK;
}


CMainApp* CMainApp::Create()
{
	CMainApp* pInstance = new CMainApp;

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Init CMainApp");
		return nullptr;
	}

	return pInstance;
}
