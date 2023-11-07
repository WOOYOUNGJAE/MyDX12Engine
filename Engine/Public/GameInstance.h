#pragma once
#include "Base.h"

NAMESPACE_(Engine)

class ENGINE_DLL CGameInstance : public CBase
{
	DECLARE_SINGLETON(CGameInstance)

private:
	CGameInstance();
	virtual ~CGameInstance() override = default;

public:
	HRESULT Free() override;

public:
	HRESULT Init_Engine();
	HRESULT Release_Engine();

private:
	class CGraphic_Device* m_pGraphic_Device = nullptr;
};

_NAMESPACE