#include "GameObject.h"

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



	return S_OK;
}
