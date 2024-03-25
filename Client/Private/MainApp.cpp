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

	/*objDesc.strTag = L"Triangle";
	objDesc.vStartPos = Vector3(0, 0, 1);
	hr = m_pGameInstance->Add_GameObject_InScene(L"Triangle", OBJ_LAYER_0, &pObjectControlling, &objDesc);
	if (FAILED(hr)) { return hr; }*/

	objDesc.strTag = L"Cube";
	objDesc.vStartPos = Vector3::Zero;
	hr = m_pGameInstance->Add_GameObject_InScene(L"Cube", OBJ_LAYER_0, &pObjectControlling, &objDesc);
	if (FAILED(hr)) { return hr; }

	objDesc.strTag = L"ZeldaDemo";
	objDesc.vStartPos = Vector3(2.f, 0.f, -5.f);
	hr = m_pGameInstance->Add_GameObject_InScene(L"ZeldaDemo", OBJ_LAYER_0, &pObjectControlling, &objDesc);
	if (FAILED(hr)) { return hr; }

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


	return S_OK;
}
void CMainApp::Tick(_float fDeltaTime)
{
	m_pRenderer->BeginRender();

	m_pGameInstance->Engine_Tick(fDeltaTime);

	m_pRenderer->MainRender();

#if IMGUI_ON
	m_pClient_Imgui->Imgui_Tick();
	m_pClient_Imgui->Imgui_MainRender();
	m_pClient_Imgui->IMgui_EndRender();
	m_pClient_Imgui->Imgui_Present();
#else
	m_pRenderer->EndRender();
	m_pRenderer->Present();
#endif

}


HRESULT CMainApp::Free()
{
#if IMGUI_ON
	Safe_Release(m_pClient_Imgui);
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
