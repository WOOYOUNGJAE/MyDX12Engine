#include "ObjLayer.h"
#include "GameObject.h"

void CObjLayer::Tick(_float fDeltaTime)
{
	for (const auto& iter : m_GameObjList)
	{
		iter->Tick(fDeltaTime);
	}

}

HRESULT CObjLayer::Late_Tick(_float fDeltaTime)
{
	for (const auto& iter : m_GameObjList)
	{
		iter->Late_Tick(fDeltaTime);
	}
	return S_OK;
}

void CObjLayer::Render_Tick()
{
	for (const auto& iter : m_GameObjList)
	{
		iter->Render_Tick();
	}
}

HRESULT CObjLayer::Free()
{
	for (auto& iter : m_GameObjList)
	{
		Safe_Release(iter);
	}

	m_GameObjList.clear();

	return S_OK;
}

HRESULT CObjLayer::Add_GameObject(CGameObject* pInstance)
{
	if (pInstance == nullptr)
	{
		MSG_BOX("Obj Layer : Instance nullptr");
		return E_FAIL;
	}

	m_GameObjList.push_back(pInstance);


	return S_OK;
}

CGameObject* CObjLayer::Find_Obj(const wstring& strPrototypeTag, UINT iClonedNum)
{
	for (auto& iter : m_GameObjList)
	{
		if (iter->Get_ClonedNum() == iClonedNum)
		{
			return iter;
		}
	}

	return nullptr;
}
