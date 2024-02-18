#include "pch.h"
#include "Camera_Free.h"
#include "Transform.h"

CCamera_Free::CCamera_Free(CCamera_Free& rhs) : CCamera(rhs)
{
}

CCamera_Free* CCamera_Free::Create()
{
	CCamera_Free* pInstance = new CCamera_Free();

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("CameraFree: Failed to Init CameraFree");
	}

	return pInstance;
}

CGameObject* CCamera_Free::Clone(void* pArg)
{
	CCamera_Free* pInstance = new CCamera_Free(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("CameraFree: Failed to Clone CameraFree");
	}

	return pInstance;
}

HRESULT CCamera_Free::Initialize_Prototype()
{
	return CCamera::Initialize_Prototype();
}

HRESULT CCamera_Free::Initialize(void* pArg)
{
	return CCamera::Initialize(pArg);
}

void CCamera_Free::Tick(_float fDeltaTime)
{
	if (g_Input->Key_Pressing(VK_UP))
	{
		m_pTransformCom->Set_Position(Vector3(m_pTransformCom->m_vPosition.x, m_pTransformCom->m_vPosition.y + 0.1f, m_pTransformCom->m_vPosition.z));
	}
	if (g_Input->Key_Pressing(VK_RIGHT))
	{
		m_pTransformCom->Set_Position(Vector3(m_pTransformCom->m_vPosition.x + 0.1f, m_pTransformCom->m_vPosition.y, m_pTransformCom->m_vPosition.z));
	}
	if (g_Input->Key_Pressing(VK_DOWN))
	{
		m_pTransformCom->Set_Position(Vector3(m_pTransformCom->m_vPosition.x, m_pTransformCom->m_vPosition.y - 0.1f, m_pTransformCom->m_vPosition.z));
	}
	if (g_Input->Key_Pressing(VK_LEFT))
	{
		m_pTransformCom->Set_Position(Vector3(m_pTransformCom->m_vPosition.x - 0.1f, m_pTransformCom->m_vPosition.y, m_pTransformCom->m_vPosition.z));
	}
}

void CCamera_Free::Late_Tick(_float fDeltaTime)
{
	CCamera::Late_Tick(fDeltaTime);
}

HRESULT CCamera_Free::Free()
{
	return CCamera::Free();
}
