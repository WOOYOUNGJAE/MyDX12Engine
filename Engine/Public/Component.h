#pragma once
#include "Base.h"
//#include "Engine_Defines.h"

NAMESPACE_(Engine)

class ENGINE_DLL CComponent abstract: CBase
{
protected:
	CComponent();
	virtual ~CComponent() override = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize();
	virtual CComponent* Clone(void* pArg) = 0;
	virtual HRESULT Free() override;
protected:
	ID3D12Device* m_pDevice = nullptr;

};

_NAMESPACE