#include "CameraManager.h"
#include "Camera.h"

IMPLEMENT_SINGLETON (CCameraManager)

HRESULT CCameraManager::Free()
{
	for (auto& iter : m_mapCamera)
	{
		Safe_Release(iter.second);
	}
	m_mapCamera.clear();

	return S_OK;
}

void CCameraManager::Set_MainCam(wstring strName)
{
	m_pMainCam = FindandGet(strName);
}

CCamera* CCameraManager::FindandGet(wstring strName)
{
	auto iter = m_mapCamera.find(strName);

	if (iter == m_mapCamera.end())
	{
		return nullptr;
	}

	return iter->second;
}

void CCameraManager::Register(wstring strName, CCamera* pInstance)
{
	if (FindandGet(strName)) // 이미 등록되었으면
	{
		return;
	}

	m_mapCamera.emplace(strName, pInstance);
	Safe_AddRef(pInstance);
}
