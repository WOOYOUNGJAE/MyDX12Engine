#pragma once
#include "Base.h"

NAMESPACE_(Engine)

class ENGINE_DLL CComponent abstract: public CBase
{
protected:
	CComponent();
	virtual ~CComponent() override = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg = nullptr);
	virtual CComponent* Clone(void* pArg) = 0;
	virtual HRESULT Free() override;
protected:
	
};

_NAMESPACE