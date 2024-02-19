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
	m_Desc = { XMConvertToRadians(60.f), g_iWinSizeX/ (FLOAT)g_iWinSizeY , 0.2f, 300.f};

	m_fMoveSpeed = 1.f;
	m_fRotSpeed = 0.5f;

	return CCamera::Initialize(pArg);
}

void CCamera_Free::Tick(_float fDeltaTime)
{

	if (g_Input->Key_Pressing('W'))
	{
		m_pTransformCom->Set_Position(
			Vector3(m_pTransformCom->m_vPosition.x,
				m_pTransformCom->m_vPosition.y + m_fMoveSpeed*fDeltaTime,
				m_pTransformCom->m_vPosition.z));
	}
	if (g_Input->Key_Pressing('D'))
	{
		m_pTransformCom->Set_Position(Vector3(
			m_pTransformCom->m_vPosition.x + m_fMoveSpeed * fDeltaTime,
			m_pTransformCom->m_vPosition.y,
			m_pTransformCom->m_vPosition.z));
	}
	if (g_Input->Key_Pressing('S'))
	{
		m_pTransformCom->Set_Position(Vector3(
			m_pTransformCom->m_vPosition.x,
			m_pTransformCom->m_vPosition.y - m_fMoveSpeed * fDeltaTime,
			m_pTransformCom->m_vPosition.z));
	}
	if (g_Input->Key_Pressing('A'))
	{
		m_pTransformCom->Set_Position(Vector3(
			m_pTransformCom->m_vPosition.x - m_fMoveSpeed * fDeltaTime,
			m_pTransformCom->m_vPosition.y,
			m_pTransformCom->m_vPosition.z));
	}

	if (g_Input->Key_Pressing(VK_RBUTTON))
	{
		const Vector2& vChangedDir = g_Input->Get_MouseDirPrevToCur();

		
		m_pTransformCom->Rotate(CTransform::AXIS_X, vChangedDir.y * m_fRotSpeed * fDeltaTime);
		m_pTransformCom->Rotate(CTransform::AXIS_Y, vChangedDir.x * m_fRotSpeed * fDeltaTime);

	}

	if (g_Input->Key_Pressing(VK_UP))
	{
		const Vector2& vChangedDir = Vector2(0, 10);


		m_pTransformCom->Rotate(CTransform::AXIS_X, -vChangedDir.y * m_fRotSpeed * fDeltaTime);
	}
	if (g_Input->Key_Pressing(VK_RIGHT))
	{
		const Vector2& vChangedDir = Vector2(10, 0);


		m_pTransformCom->Rotate(CTransform::AXIS_Y, vChangedDir.x * m_fRotSpeed * fDeltaTime);
	}
	if (g_Input->Key_Pressing(VK_DOWN))
	{
		const Vector2& vChangedDir = Vector2(0, -10);


		m_pTransformCom->Rotate(CTransform::AXIS_X, -vChangedDir.y * m_fRotSpeed * fDeltaTime);
	}
	if (g_Input->Key_Pressing(VK_LEFT))
	{
		const Vector2& vChangedDir = Vector2(-10, 0);


		m_pTransformCom->Rotate(CTransform::AXIS_Y, vChangedDir.x * m_fRotSpeed * fDeltaTime);
	}



	if (g_Input->Key_Down('X'))
	{
		m_pTransformCom->Set_Look(Vector3(0.f, 0.f, 1.f));
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
