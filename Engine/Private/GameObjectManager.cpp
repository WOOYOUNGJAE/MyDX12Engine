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
	return S_OK;
}

HRESULT CGameObjectManager::Late_Tick(_float fDeltaTime)
{
	return S_OK;
}

CGameObject* CGameObjectManager::Find_Prototype(const wstring& strTag)
{
}

HRESULT CGameObjectManager::Add_Prototype(const wstring& strTag, CGameObject* pInstance)
{
	return S_OK;
}

CGameObject* CGameObjectManager::Clone_GameObject(const wstring& strTag, void* pArg)
{
}
