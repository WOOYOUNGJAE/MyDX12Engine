#include "Camera.h"
#include "Pipeline.h"
#include "Transform.h"

CCamera::CCamera() : m_pPipeline(CPipeline::Get_Instance())
{
	Safe_AddRef(m_pPipeline);
}

HRESULT CCamera::Free()
{
	Safe_Release(m_pTransformCom);
	Safe_Release(m_pPipeline);
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
	m_pPipeline->Update_Matrix(m_pTransformCom->WorldMatrix_Inverse(), CPipeline::VIEW_MAT);

	m_pPipeline->Update_Matrix(XMMatrixPerspectiveFovLH(m_fFovy, m_fAspect, m_fNear, m_fFar), CPipeline::PROJ_MAT);
}
