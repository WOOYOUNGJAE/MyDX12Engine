#include "CameraManager.h"
#include "Camera.h"
#include "ComponentManager.h"
#include "DXRRenderer.h"
#include "Renderer.h"


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

#if DXR_ON
void CCameraManager::Set_MainCam(wstring strName, CDXRRenderer* pDXRRenderer)
{
	m_pMainCam = FindandGet(strName);

	CRenderer* pRenderer = CComponentManager::Get_Instance()->Get_Renderer();

	if (pRenderer == nullptr)
	{
		MSG_BOX("CamManager : SetMainCam Failed, renderer nullptr");
	}

	pRenderer->Set_ProjMat(m_pMainCam->Get_CamDesc());

	pDXRRenderer->Update_Static_PassCB(m_pMainCam->Get_CamDesc());
}
#else
void CCameraManager::Set_MainCam(wstring strName)
{
	m_pMainCam = FindandGet(strName);

	CRenderer* pRenderer = CComponentManager::Get_Instance()->Get_Renderer();

	if (pRenderer == nullptr)
	{
		MSG_BOX("CamManager : SetMainCam Failed, renderer nullptr");
	}

	pRenderer->Set_ProjMat(m_pMainCam->Get_CamDesc());
}
#endif

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
