#pragma once
#include "Camera.h"
#include "Client_Defines.h"

NAMESPACE_(Client)
class CCamera_Free : public CCamera
{
private:
	CCamera_Free() = default;
	~CCamera_Free() override = default;

public:
	static CCamera_Free* Create();
	CGameObject* Clone(void* pArg) override;
	HRESULT Initialize_Prototype() override;
	HRESULT Initialize(void* pArg) override;
	void Tick(_float fDeltaTime) override;
	void Late_Tick(_float fDeltaTime) override;
	HRESULT Free() override;
};

_NAMESPACE