#pragma once
#include "Component.h"

NAMESPACE_(Engine)

class ENGINE_DLL CTransform final : public CComponent
{
private:
	CTransform() = default;
	CTransform(CTransform& rhs) = default;
	~CTransform() override = default;

public:
	CComponent* Clone(void* pArg) override;
private:
	_float4x4 m_WorldMatrix;
};

_NAMESPACE