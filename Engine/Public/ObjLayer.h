#pragma once
#include "Base.h"
NAMESPACE_(Engine)
class CGameObject;
// Layer 오브젝트, 리스트 형태
// 레이어 는 오브젝트 여러종류 가질 수 있음
// 오브젝트는 단 하나의 레이어에만
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
	const std::list<CGameObject*>& Get_ObjList() const { return m_GameObjList; }
public:
	HRESULT Add_GameObject(CGameObject* pInstance);
	CGameObject* Find_Obj(const wstring& strPrototypeTag, UINT iClonedNum = 1);
public:
	std::list<CGameObject*> m_GameObjList;


};

_NAMESPACE