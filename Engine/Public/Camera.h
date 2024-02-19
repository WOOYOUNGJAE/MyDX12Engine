#pragma once
#include "GameObject.h"

NAMESPACE_(Engine)
// Client에서 상속받아서 구현
class ENGINE_DLL CCamera abstract: public CGameObject
{
protected:
	CCamera();
	CCamera(CCamera& rhs);
	~CCamera() override = default;

public:
	HRESULT Initialize_Prototype() override;
	HRESULT Initialize(void* pArg) override;
	HRESULT Free() override;
	void Tick(_float fDeltaTime) override;
	void Late_Tick(_float fDeltaTime) override;
public:
	const CAMERA_DESC& Get_CamDesc() const { return m_Desc; }
	Vector3 Get_Pos() override;
	Matrix Get_WorldMatrix() override;
protected: // pointer
	class CTransform* m_pTransformCom = nullptr;
protected:
	CAMERA_DESC m_Desc =
	{ 0.f, 0.f, 0.f};
};

_NAMESPACE