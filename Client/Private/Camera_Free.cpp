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
	CCamera::Initialize(pArg);

	m_Desc = { XMConvertToRadians(60.f), g_iWinSizeX/ (FLOAT)g_iWinSizeY , 0.2f, 300.f};

	m_fMoveSpeed = 1.f;
	m_fRotSpeed = 0.5f;

	return S_OK;
}

void CCamera_Free::Tick(_float fDeltaTime)
{

	if (g_Input->Key_Pressing('W'))
	{
		m_pTransformCom->Move_Forward(m_fMoveSpeed * fDeltaTime);
	}
	if (g_Input->Key_Pressing('D'))
	{
		m_pTransformCom->Move_Right(m_fMoveSpeed * fDeltaTime);
	}
	if (g_Input->Key_Pressing('S'))
	{
		m_pTransformCom->Move_Forward(-m_fMoveSpeed * fDeltaTime);
	}
	if (g_Input->Key_Pressing('A'))
	{
		m_pTransformCom->Move_Right(-m_fMoveSpeed * fDeltaTime);
	}
	if (g_Input->Key_Pressing('Q'))
	{
		m_pTransformCom->Move_Up(m_fMoveSpeed * fDeltaTime);
	}
	if (g_Input->Key_Pressing('E'))
	{
		m_pTransformCom->Move_Up(-m_fMoveSpeed * fDeltaTime);		
	}

	if (g_Input->Key_Pressing(VK_RBUTTON))
	{
		const Vector2& vChangedDir = g_Input->Get_MouseDirPrevToCur();

		// ���� �������� y���� �������� ȸ�� �ؾ� ��
		m_pTransformCom->Rotate(Vector3::Up, vChangedDir.x * m_fRotSpeed * fDeltaTime);
		m_pTransformCom->Rotate(m_pTransformCom->WorldMatrix().Right(), vChangedDir.y * m_fRotSpeed * fDeltaTime);

	}



	if (g_Input->Key_Down('X')) // ī�޶� ȸ�� �ǵ�����
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
