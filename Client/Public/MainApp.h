#pragma once
#include "Base.h"
#include "Client_Defines.h"
NAMESPACE_(Engine)
class CGameInstance;
_NAMESPACE

NAMESPACE_(Client)
class CMainApp : public CBase
{
private:
	CMainApp();
	virtual ~CMainApp() override;

public:
	static CMainApp* Create();
	HRESULT Initialize();
	virtual HRESULT Free() override;

private:
	ID3D12Device* m_pDevice = nullptr;
	CGameInstance* m_pGameInstance = nullptr;
};

_NAMESPACE