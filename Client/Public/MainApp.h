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
	void Tick(_float fDeltaTime);
	void Late_Tick(_float fDeltaTime);
	void Render();
	virtual HRESULT Free() override;

private:
	ID3D12Device* m_pDevice = nullptr;
	CGameInstance* m_pGameInstance = nullptr;
};

_NAMESPACE