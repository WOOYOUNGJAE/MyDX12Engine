#include "pch.h"
#include "MainApp.h"
#include "GameInstance.h"
CMainApp::CMainApp()
{
}

CMainApp::~CMainApp()
{
}

HRESULT CMainApp::Initialize()
{
	return S_OK;
}

HRESULT CMainApp::Free()
{
	Safe_Release(m_pGameInstance);
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

	;
	return pInstance;
}
