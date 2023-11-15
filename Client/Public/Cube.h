#pragma once
#include "GameObject.h"
#include "Client_Defines.h"

NAMESPACE_(Engine)
class CCubeMesh;
_NAMESPACE

NAMESPACE_(Client)
class CCube : public CGameObject
{
protected:
	CCube() = default;
	CCube(const CCube& rhs): CGameObject(rhs){}
	~CCube() override = default;

public:
	static CCube* Create();
	CGameObject* Clone(void* pArg) override;
	HRESULT Initialize_Prototype() override;
	HRESULT Initialize(void* pArg) override;
	void Tick(_float fDeltaTime) override;
	void Late_Tick(_float fDeltaTime) override;
	void Render() override;
	HRESULT Free() override;

protected:
	CCubeMesh* m_pCubeMeshCom = nullptr;
};

_NAMESPACE