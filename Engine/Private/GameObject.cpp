#include "GameObject.h"
#include "GameInstance.h"
#include "Renderer.h"

HRESULT CGameObject::Free()
{
	return S_OK;
}

_bool CGameObject::Com_Already_Owned(const wstring& strComTag)
{
	auto iter = m_mapComponents.find(strComTag);

	if (iter == m_mapComponents.end()) // 없음
	{
		return false;
	}

	return true; // 이미 있음
}

HRESULT CGameObject::Add_Component(const wstring& strComTag, CComponent** ppOutCom, void* pArg)
{
	// 이미 갖고 있는 컴포넌트라면
	if (Com_Already_Owned(strComTag))
	{
		return S_OK; // 크래시는 일단 안함
	}

	CGameInstance* pGameInstance = CGameInstance::Get_Instance();
	Safe_AddRef(pGameInstance);

	CComponent* pComponent = pGameInstance->Clone_ComPrototype(strComTag, pArg);
	if (pComponent == nullptr)
	{
		return E_FAIL;
	}


	m_mapComponents.emplace(strComTag, pComponent);

	*ppOutCom = pComponent;

	Safe_Release(pGameInstance);
	return S_OK;
}
