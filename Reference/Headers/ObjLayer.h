#pragma once
#include "Base.h"
// Layer 오브젝트, 리스트 형태
NAMESPACE_(Engine)
class CGameObject;
class CObjLayer : public CBase
{
protected:
	CObjLayer() = default;
	~CObjLayer() override = default;

public:
	static CObjLayer* Create() { return new CObjLayer; }
	void Tick(_float fDeltaTime);
	HRESULT Late_Tick(_float fDeltaTime);
	virtual void Render_Tick();
	HRESULT Free() override;
public:
	HRESULT Add_GameObject(CGameObject* pInstance);

public:
	std::list<CGameObject*> m_GameObjList;


};

_NAMESPACE