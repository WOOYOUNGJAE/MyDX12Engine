#include "Camera.h"
#include "PipelineManager.h"
#include "Transform.h"
#include "CameraManager.h"

CCamera::CCamera()
{
}
CCamera::CCamera(CCamera& rhs) : CGameObject(rhs)
{
}

HRESULT CCamera::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CCamera::Initialize(void* pArg)
{
	HRESULT hr = Add_Component(L"Transform", reinterpret_cast<CComponent**>(&m_pTransformCom));

	CCameraManager::Get_Instance()->Register(*reinterpret_cast<wstring*>(pArg), this);

	return hr;
}

HRESULT CCamera::Free()
{
	Safe_Release(m_pTransformCom);
	return CGameObject::Free();
}

void CCamera::Tick(_float fDeltaTime)
{
}

void CCamera::Late_Tick(_float fDeltaTime)
{
}

Vector3 CCamera::Get_Pos()
{
	return m_pTransformCom->Position();
}

Matrix CCamera::Get_WorldMatrix()
{
	return m_pTransformCom->m_mWorldMatrix;
}
