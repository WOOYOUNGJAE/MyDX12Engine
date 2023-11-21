#pragma once
#include "GameObject.h"

NAMESPACE_(Engine)

class ENGINE_DLL CCube : public CGameObject
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
	void Pipeline_Tick() override;
	void Render() override;
	HRESULT Free() override;

protected:
	class CTransform* m_pTransform = nullptr;
	class CCubeMesh* m_pCubeMeshCom = nullptr;

#ifdef _DEBUG
	static _uint m_iClonedNum;
#endif // DEBUG
};

_NAMESPACE