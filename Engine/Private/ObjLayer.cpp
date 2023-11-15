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

HRESULT CObjLayer::Free()
{
	for (const auto& iter : m_GameObjList)
	{
		Safe_Release(iter);
	}

	m_GameObjList.clear();

	return S_OK;
}
