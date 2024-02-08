#include "pch.h"
#include "MainApp.h"

#include "Renderer.h"
#include "Client_Defines.h"
#include "GameInstance.h"
#include "Camera_Free.h"

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

	CGameObject* pObjectControlling = nullptr;

	HRESULT hr = 
	m_pGameInstance->Init_Engine(graphic_desc, &m_pDevice);
	if (FAILED(hr)) { return hr; }
#pragma region InLevel
	hr = m_pGameInstance->Add_GameObjPrototype(L"Camera_Free", CCamera_Free::Create());
	if (FAILED(hr)) { return hr; }
	hr = m_pGameInstance->Add_GameObject_InScene(L"Camera_Free", L"Default");
	if (FAILED(hr)) { return hr; }
	hr = m_pGameInstance->Add_GameObject_InScene(L"Triangle", L"Layer0", &pObjectControlling);
	if (FAILED(hr)) { return hr; }
	/*hr = m_pGameInstance->Add_GameObject_InScene(L"Cube", L"Layer0", &pObjectControlling);
	if (FAILED(hr)) { return hr; }*/
	// m_pGameInstance->Update_ObjPipelineLayer(pObjectControlling, Pipeline::ENUM_PSO::PSO_DEFAULT);
#pragma endregion InLevel

	// Renderer
	m_pRenderer = m_pGameInstance->Get_Instance()->Get_Renderer();

	if (m_pRenderer == nullptr)
	{
		MSG_BOX("MainApp: Renderer is nullptr");
		return E_FAIL;
	}



	return S_OK;
}
void CMainApp::Tick(_float fDeltaTime)
{
	m_pRenderer->BeginRender();

	m_pGameInstance->Engine_Tick(fDeltaTime);

	m_pRenderer->MainRender();
	m_pRenderer->EndRender();
	m_pRenderer->Present();
}

HRESULT CMainApp::Free()
{
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
