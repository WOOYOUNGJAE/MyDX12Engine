#include "GameObjectManager.h"
#include "GameObject.h"
#include "ObjLayer.h"

HRESULT CGameObjectManager::Initialize()
{
	return S_OK;
}

HRESULT CGameObjectManager::Free()
{
	return S_OK;
}

HRESULT CGameObjectManager::Tick(_float fDeltaTime)
{
	for (const auto& pair : m_mapLayer)
	{
		pair.second->Tick(fDeltaTime);
	}
	return S_OK;
}

HRESULT CGameObjectManager::Late_Tick(_float fDeltaTime)
{
	for (const auto& pair : m_mapLayer)
	{
		pair.second->Late_Tick(fDeltaTime);
	}
	return S_OK;
}

CGameObject* CGameObjectManager::Find_Prototype(const wstring& strTag)
{
	auto pair = m_mapObjPrototypes.find(strTag);

	if (pair == m_mapObjPrototypes.end())
	{
		return nullptr;
	}

	return pair->second;
}

CObjLayer* CGameObjectManager::Find_Layer(const wstring& strTag)
{
	auto pair = m_mapLayer.find(strTag);

	if (pair == m_mapLayer.end())
	{
		return nullptr;
	}

	return pair->second;
}

HRESULT CGameObjectManager::Add_Prototype(const wstring& strTag, CGameObject* pInstance)
{
	// �̹� ������ ��
	if (Find_Prototype(strTag))
	{
		MSG_BOX("GameObj Manager : Prototype Already Exists");
		return E_FAIL;
	}

	m_mapObjPrototypes.emplace(strTag, pInstance);

	return S_OK;
}

CGameObject* CGameObjectManager::Clone_GameObject(const wstring& strTag, void* pArg)
{
	CGameObject* pPrototype = Find_Prototype(strTag);

	// �������� ���� ��
	if (pPrototype == nullptr)
	{
		MSG_BOX("GameObjectManager : Prototype Doesn't Exist");
		return nullptr;
	}

	return pPrototype->Clone(pArg);
}

HRESULT CGameObjectManager::Add_GameObject_InScene(const wstring& strPrototypeTag, const wstring& strLayerTag,
	void* pArg)
{
	auto prototypePair = m_mapObjPrototypes.find(strPrototypeTag);

	// �� ã���� ��
	if (prototypePair == m_mapObjPrototypes.end())
	{
		MSG_BOX("GameObjectMananger : Prototype Doesn't Exist");
		return E_FAIL;
	}

	CGameObject* pGameObject = prototypePair->second->Clone(pArg);

	CObjLayer* pLayer = Find_Layer(strLayerTag);

	// ó�� ���� ���̾��� �߰�
	if (pLayer == nullptr)
	{
		pLayer = CObjLayer::Create();
		pLayer->Add_GameObject(pGameObject); // ���̾ �ְ�
		m_mapLayer.emplace(strLayerTag, pLayer); // ���̾� ��ü�� �߰�

		return S_OK;
	}

	// �̹� �����ϴ� ���̾ �߰�
	pLayer->Add_GameObject(pGameObject);

	return S_OK;
}