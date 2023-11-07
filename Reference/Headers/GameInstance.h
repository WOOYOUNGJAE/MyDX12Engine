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
};

_NAMESPACE