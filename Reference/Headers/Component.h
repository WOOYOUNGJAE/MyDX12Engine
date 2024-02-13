#pragma once
#include "Base.h"

NAMESPACE_(Engine)

class ENGINE_DLL CComponent abstract: public CBase
{
protected:
	CComponent();
	CComponent(CComponent& rhs) : CBase(rhs){}
	virtual ~CComponent() override = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg = nullptr);
	virtual CComponent* Clone(void* pArg = nullptr) = 0;
	virtual HRESULT Free() override;

public:
	UINT Get_DescriptorCount() const { return m_iDescriptorCount; }

protected:
	UINT m_iDescriptorCount = 0;
};

_NAMESPACE