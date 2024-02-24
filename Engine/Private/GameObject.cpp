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

	if (iter == m_mapComponents.end()) // ����
	{
		return false;
	}

	return true; // �̹� ����
}

HRESULT CGameObject::Add_Component(const wstring& strComTag, CComponent** ppOutCom, void* pArg)
{
	// �̹� ���� �ִ� ������Ʈ���
	if (Com_Already_Owned(strComTag))
	{
		return S_OK; // ũ���ô� �ϴ� ����
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
