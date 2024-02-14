#pragma once
#include "Base.h"
#include "Client_Defines.h"
NAMESPACE_(Engine)
class CGameInstance;
class CRenderer;
_NAMESPACE

NAMESPACE_(Client)
class CClient_Imgui;
class CMainApp : public CBase
{
private:
	CMainApp();
	virtual ~CMainApp() override;

public:
	static CMainApp* Create();
	HRESULT Initialize();
	void Tick(_float fDeltaTime);
	virtual HRESULT Free() override;

private:
	ID3D12Device* m_pDevice = nullptr;
	CGameInstance* m_pGameInstance = nullptr;
	CRenderer* m_pRenderer = nullptr;
	CClient_Imgui* m_pClient_Imgui = nullptr;
};

_NAMESPACE