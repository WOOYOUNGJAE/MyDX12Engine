#pragma once
#include "Camera.h"
#include "Client_Defines.h"

NAMESPACE_(Client)
using namespace Engine;
class CCamera_Free : public CCamera
{
private:
	CCamera_Free() = default;
	CCamera_Free(CCamera_Free& rhs);
	~CCamera_Free() override = default;

public:
	static CCamera_Free* Create();
	CGameObject* Clone(void* pArg) override;
	HRESULT Initialize_Prototype() override;
	HRESULT Initialize(void* pArg) override;
	void Tick(_float fDeltaTime) override;
	void Late_Tick(_float fDeltaTime) override;
	HRESULT Free() override;
	XMFLOAT4X4 Get_WorldMatrix() override { return XMFLOAT4X4(); };
};

_NAMESPACE