#include "Camera.h"
#include "PipelineManager.h"
#include "Transform.h"
#include "CameraManager.h"

CCamera::CCamera() : m_pPipelineManager(CPipelineManager::Get_Instance())
{
	Safe_AddRef(m_pPipelineManager);
}
CCamera::CCamera(CCamera& rhs) : CGameObject(rhs),
m_pPipelineManager(rhs.m_pPipelineManager)
{
	Safe_AddRef(m_pPipelineManager);
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
	Safe_Release(m_pPipelineManager);
	return CGameObject::Free();
}

void CCamera::Tick(_float fDeltaTime)
{
}

void CCamera::Late_Tick(_float fDeltaTime)
{
	Update_PipelineView();
}

void CCamera::Update_PipelineView()
{
	// m_pPipelineManager->Update_Matrix(m_pTransformCom->WorldMatrix_Inverse(), CPipelineManager::VIEW_MAT);

	// m_pPipelineManager->Update_Matrix(XMMatrixPerspectiveFovLH(m_fFovy, m_fAspect, m_fNear, m_fFar), CPipelineManager::PROJ_MAT);
}

Vector3 CCamera::Get_Pos()
{
	return m_pTransformCom->Position();
}

Matrix CCamera::Get_WorldMatrix()
{
	return m_pTransformCom->m_mWorldMatrix;
}
