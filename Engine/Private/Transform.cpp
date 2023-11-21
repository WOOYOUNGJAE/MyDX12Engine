#include "Transform.h"

CTransform* CTransform::Create()
{
	CTransform* pInstance = new CTransform();

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Transform: Failed to Create Transform");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CComponent* CTransform::Clone(void* pArg)
{
	CTransform* pInstance = new CTransform(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Transform: Failed to Clone Transform");
		Safe_Release(pInstance);
	}

	return pInstance;
}

HRESULT CTransform::Initialize_Prototype()
{
	if (FAILED(CComponent::Initialize_Prototype()))
	{
		return E_FAIL;
	}

	XMStoreFloat4x4(&m_WorldMatrix, XMMatrixIdentity());

	return S_OK;
}

HRESULT CTransform::Initialize(void* pArg)
{
	return S_OK;
}


HRESULT CTransform::Free()
{
	return CComponent::Free();
}

void CTransform::Set_WorldMatrix(MATRIX_ENUM eEnum, _fvector vVec)
{
	_matrix		WorldMatrix = XMLoadFloat4x4(&m_WorldMatrix);

	WorldMatrix.r[eEnum] = vVec;

	XMStoreFloat4x4(&m_WorldMatrix, WorldMatrix);
}

void CTransform::Refresh_Transform(TRANSFORM_ENUM eEnum)
{
	switch (eEnum)
	{
	case LOOK:
		{
		_vector vLook = XMLoadFloat3(&m_Look);
		_vector vRight = XMVector3Cross(XMVectorSet(0.f, 1.f, 0.f, 0.f), vLook);
		_vector	vUp = XMVector3Cross(vLook, vRight);

		_float3 ScaleLength = Get_ScaleXYZ(); // 기존 스케일 저장

		Set_WorldMatrix(MAT_RIGHT, vRight);
		Set_WorldMatrix(MAT_UP, vUp);
		Set_WorldMatrix(MAT_LOOK, vLook);

		Refresh_MatrixScaled(ScaleLength); // 변형된 스케일 정상화
		}
		break;
	case POSITION:
		Set_WorldMatrix(MAT_POSITION, XMLoadFloat3(&m_Position));
		break;
	case SCALE:
		Refresh_MatrixScaled(m_Scale);
		break;
	default: // Scale Look Position 모두 업데이트
		{
		Refresh_MatrixScaled(m_Scale);

		_vector vLook = XMLoadFloat3(&m_Look);
		_vector vRight = XMVector3Cross(XMVectorSet(0.f, 1.f, 0.f, 0.f), vLook);
		_vector	vUp = XMVector3Cross(vLook, vRight);

		_float3 ScaleLength = Get_ScaleXYZ(); // 기존 스케일 저장

		Set_WorldMatrix(MAT_RIGHT, vRight);
		Set_WorldMatrix(MAT_UP, vUp);
		Set_WorldMatrix(MAT_LOOK, vLook);

		Refresh_MatrixScaled(ScaleLength); // 변형된 스케일 정상화
		Set_WorldMatrix(MAT_POSITION, XMLoadFloat3(&m_Position));
		}
		break;
	}
}

_vector CTransform::Get_MatrixRow(MATRIX_ENUM eEnum)
{
	_matrix WorldMatrix = XMLoadFloat4x4(&m_WorldMatrix);

	return WorldMatrix.r[eEnum];
}

_float3 CTransform::Get_ScaleXYZ()
{
	return _float3(
		 XMVectorGetX(XMVector3Length(Get_MatrixRow(MAT_RIGHT))) ,
		 XMVectorGetX(XMVector3Length(Get_MatrixRow(MAT_UP))) ,
		 XMVectorGetX(XMVector3Length(Get_MatrixRow(MAT_LOOK)))
	);
}

void CTransform::Refresh_MatrixScaled(const _float3& scale)
{
	Set_WorldMatrix(MAT_RIGHT, XMVector3Normalize(Get_MatrixRow(MAT_RIGHT) * scale.x));
	Set_WorldMatrix(MAT_UP, XMVector3Normalize(Get_MatrixRow(MAT_UP) * scale.y));
	Set_WorldMatrix(MAT_LOOK, XMVector3Normalize(Get_MatrixRow(MAT_LOOK) * scale.z));
}
