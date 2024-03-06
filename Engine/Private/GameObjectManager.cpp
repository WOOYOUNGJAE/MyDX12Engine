#include "GameObjectManager.h"
#include "Cube.h"
#include "Grid.h"
#include "ObjLayer.h"
#include "Triangle.h"
IMPLEMENT_SINGLETON(CGameObjectManager)

HRESULT CGameObjectManager::Initialize()
{
#pragma region Init_Basic_GameObject
	Add_Prototype(L"Cube", CCube::Create());
	Add_Prototype(L"Triangle", CTriangle::Create());
	Add_Prototype(L"Grid_1X1", CGrid::Create(L"Grid_1X1"));
	Add_Prototype(L"Grid_10X10", CGrid::Create(L"Grid_10X10"));
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

void CGameObjectManager::Render_Tick()
{
	for (const auto& pair : m_mapLayer)
	{
		pair.second->Render_Tick();
	}
}

CGameObject* CGameObjectManager::FindandGet_Prototype(const wstring& strTag)
{
	auto pair = m_mapObjPrototypes.find(strTag);

	if (pair == m_mapObjPrototypes.end())
	{
		return nullptr;
	}

	return pair->second;
}

CGameObject* CGameObjectManager::FindandGet_Cloned(const wstring& strProtoTag, UINT eLayerEnum, UINT iClonedNum)
{
	CObjLayer* pLayer = Find_Layer(eLayerEnum);
	if (pLayer == nullptr) { return nullptr; }

	return pLayer->Find_Obj(strProtoTag, iClonedNum);
}

CObjLayer* CGameObjectManager::Find_Layer(UINT eLayerEnum)
{
	auto pair = m_mapLayer.find(eLayerEnum);

	if (pair == m_mapLayer.end())
	{
		return nullptr;
	}

	return pair->second;
}

HRESULT CGameObjectManager::Add_Prototype(const wstring& strTag, CGameObject* pInstance)
{
	// 이미 존재할 때
	if (FindandGet_Prototype(strTag))
	{
		MSG_BOX("GameObj Manager : Prototype Already Exists");
		return E_FAIL;
	}

	pInstance->Set_PrototypeTag(strTag);

	m_mapObjPrototypes.emplace(strTag, pInstance);

	return S_OK;
}

CGameObject* CGameObjectManager::Clone_GameObject(const wstring& strTag, void* pArg)
{
	CGameObject* pPrototype = FindandGet_Prototype(strTag);

	// 존재하지 않을 떄
	if (pPrototype == nullptr)
	{
		MSG_BOX("GameObjectManager : Prototype Doesn't Exist");
		return nullptr;
	}

	return pPrototype->Clone(pArg);
}

HRESULT CGameObjectManager::Add_GameObject_InScene(const wstring& strPrototypeTag, UINT eLayerEnum,
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

	CObjLayer* pLayer = Find_Layer(eLayerEnum);

	// 처음 보는 레이어라면 추가
	if (pLayer == nullptr)
	{
		pLayer = CObjLayer::Create();
		pLayer->Add_GameObject(pGameObject); // 레이어에 넣고
		m_mapLayer.emplace(eLayerEnum, pLayer); // 레이어 자체를 추가

		return S_OK;
	}

	// 이미 존재하는 레이어에 추가
	pLayer->Add_GameObject(pGameObject);

	return S_OK;
}

HRESULT CGameObjectManager::Add_GameObject_InScene(const wstring& strPrototypeTag, UINT eLayerEnum,
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

	CObjLayer* pLayer = Find_Layer(eLayerEnum);

	// 처음 보는 레이어라면 추가
	if (pLayer == nullptr)
	{
		pLayer = CObjLayer::Create();
		pLayer->Add_GameObject(pGameObject); // 레이어에 넣고
		m_mapLayer.emplace(eLayerEnum, pLayer); // 레이어 자체를 추가

		*pOutObj = pGameObject;

		return S_OK;
	}

	// 이미 존재하는 레이어에 추가
	pLayer->Add_GameObject(pGameObject);

	*pOutObj = pGameObject;

	return S_OK;
}
