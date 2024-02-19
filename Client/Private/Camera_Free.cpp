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
	return __super::Initialize_Prototype();
}

HRESULT CCamera_Free::Initialize(void* pArg)
{
	m_Desc = { XMConvertToRadians(60.f), 0.2f, 300.f };

	m_fMovingSpeed = 1.f;
	m_fRotatingSpeed = 1.f;

	return CCamera::Initialize(pArg);
}

void CCamera_Free::Tick(_float fDeltaTime)
{

	if (g_Input->Key_Pressing('W'))
	{
		m_pTransformCom->Set_Position(
			Vector3(m_pTransformCom->m_vPosition.x,
				m_pTransformCom->m_vPosition.y + m_fMovingSpeed*fDeltaTime,
				m_pTransformCom->m_vPosition.z));
	}
	if (g_Input->Key_Pressing('D'))
	{
		m_pTransformCom->Set_Position(Vector3(
			m_pTransformCom->m_vPosition.x + m_fMovingSpeed * fDeltaTime,
			m_pTransformCom->m_vPosition.y,
			m_pTransformCom->m_vPosition.z));
	}
	if (g_Input->Key_Pressing('S'))
	{
		m_pTransformCom->Set_Position(Vector3(
			m_pTransformCom->m_vPosition.x,
			m_pTransformCom->m_vPosition.y - m_fMovingSpeed * fDeltaTime,
			m_pTransformCom->m_vPosition.z));
	}
	if (g_Input->Key_Pressing('A'))
	{
		m_pTransformCom->Set_Position(Vector3(
			m_pTransformCom->m_vPosition.x - m_fMovingSpeed * fDeltaTime,
			m_pTransformCom->m_vPosition.y,
			m_pTransformCom->m_vPosition.z));
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
