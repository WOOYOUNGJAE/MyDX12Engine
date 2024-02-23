#include "pch.h"
#include "MainApp.h"
#include "Renderer.h"
#include "Client_Defines.h"
#include "GameInstance.h"
#include "Camera_Free.h"
#include "ClientLoader.h"

#ifdef IMGUI_ON
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



#pragma region InLevel
	CGameObject* pObjectControlling = nullptr;
	GAMEOBJECT_INIT_DESC objDesc{};
	hr = m_pGameInstance->Add_GameObjPrototype(L"Camera_Free", CCamera_Free::Create());
	if (FAILED(hr)) { return hr; }


	objDesc.strTag = L"Camera_Free";
	objDesc.vStartPos = Vector3(0.f, 2.f, -5.f);
	hr = m_pGameInstance->Add_GameObject_InScene(L"Camera_Free", OBJ_LAYER_DEFAULT,	&objDesc);
	if (FAILED(hr)) { return hr; }

	m_pGameInstance->Set_MainCam(objDesc.strTag);

	/*objDesc.strTag = L"Triangle";
	objDesc.vStartPos = Vector3(0, 0, 1);
	hr = m_pGameInstance->Add_GameObject_InScene(L"Triangle", OBJ_LAYER_0, &pObjectControlling, &objDesc);
	if (FAILED(hr)) { return hr; }*/

	objDesc.strTag = L"Cube";
	objDesc.vStartPos = Vector3(0.f, 0.f, 0.f);
	hr = m_pGameInstance->Add_GameObject_InScene(L"Cube", OBJ_LAYER_0, &pObjectControlling, &objDesc);
	if (FAILED(hr)) { return hr; }
#pragma endregion InLevel

	// Renderer
	m_pRenderer = m_pGameInstance->Get_Instance()->Get_Renderer();
	Safe_AddRef(m_pRenderer);

	if (m_pRenderer == nullptr)
	{
		MSG_BOX("MainApp: Renderer is nullptr");
		return E_FAIL;
	}

#ifdef IMGUI_ON
	m_pClient_Imgui = CClient_Imgui::Create(m_pDevice);
#endif


	return S_OK;
}
void CMainApp::Tick(_float fDeltaTime)
{
	m_pRenderer->BeginRender();

	m_pGameInstance->Engine_Tick(fDeltaTime);

	m_pRenderer->MainRender();

#ifdef IMGUI_ON
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
#ifdef IMGUI_ON
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
