#pragma once
#include "Base.h"

NAMESPACE_(Engine)
class CGameInstance;
_NAMESPACE

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
	CGameInstance* m_pGameInstance = nullptr;
};

