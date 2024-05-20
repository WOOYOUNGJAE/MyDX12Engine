#pragma once
#include "Base.h"
NAMESPACE_(Engine)
class CGameObject;
// Layer ������Ʈ, ����Ʈ ����
// ���̾� �� ������Ʈ �������� ���� �� ����
// ������Ʈ�� �� �ϳ��� ���̾��
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