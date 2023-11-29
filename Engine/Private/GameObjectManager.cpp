#include "GameObjectManager.h"
#include "Cube.h"
#include "ObjLayer.h"

IMPLEMENT_SINGLETON(CGameObjectManager)

HRESULT CGameObjectManager::Initialize()
{
#pragma region Init_Basic_GameObject
	Add_Prototype(L"Cube", CCube::Create());
#pragma endregion Init_Basic_GameObject
	return S_OK;
}

HRESULT CGameObjectManager::Free()
{
	for (auto& pair : m_mapObjPrototypes)
	{
		Safe_Release(pair.second);
	}
	m_mapObjPrototypes.clear();

	for (auto& pair : m_mapLayer)
	{
		Safe_Release(pair.second);
	}
	m_mapLayer.clear();

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
	// 이미 존재할 때
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

	// 존재하지 않을 떄
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

	// 못 찾았을 때
	if (prototypePair == m_mapObjPrototypes.end())
	{
		MSG_BOX("GameObjectMananger : Prototype Doesn't Exist");
		return E_FAIL;
	}

	CGameObject* pGameObject = prototypePair->second->Clone(pArg);

	CObjLayer* pLayer = Find_Layer(strLayerTag);

	// 처음 보는 레이어라면 추가
	if (pLayer == nullptr)
	{
		pLayer = CObjLayer::Create();
		pLayer->Add_GameObject(pGameObject); // 레이어에 넣고
		m_mapLayer.emplace(strLayerTag, pLayer); // 레이어 자체를 추가

		return S_OK;
	}

	// 이미 존재하는 레이어에 추가
	pLayer->Add_GameObject(pGameObject);

	return S_OK;
}

HRESULT CGameObjectManager::Add_GameObject_InScene(const wstring& strPrototypeTag, const wstring& strLayerTag,
	CGameObject** pOutObj, void* pArg)
{
	auto prototypePair = m_mapObjPrototypes.find(strPrototypeTag);

	// 못 찾았을 때
	if (prototypePair == m_mapObjPrototypes.end())
	{
		MSG_BOX("GameObjectMananger : Prototype Doesn't Exist");
		return E_FAIL;
	}

	CGameObject* pGameObject = prototypePair->second->Clone(pArg);

	CObjLayer* pLayer = Find_Layer(strLayerTag);

	// 처음 보는 레이어라면 추가
	if (pLayer == nullptr)
	{
		pLayer = CObjLayer::Create();
		pLayer->Add_GameObject(pGameObject); // 레이어에 넣고
		m_mapLayer.emplace(strLayerTag, pLayer); // 레이어 자체를 추가

		*pOutObj = pGameObject;

		return S_OK;
	}

	// 이미 존재하는 레이어에 추가
	pLayer->Add_GameObject(pGameObject);

	*pOutObj = pGameObject;

	return S_OK;
}
