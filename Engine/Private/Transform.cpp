#include "Transform.h"
#include "Shader.h"
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
	Matrix_Identity(m_mWorldMatrix);
	//XMStoreFloat4x4(&m_mWorldMatrix, XMMatrixIdentity());

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
	_matrix		WorldMatrix = XMLoadFloat4x4(&m_mWorldMatrix);

	WorldMatrix.r[eEnum] = vVec;

	XMStoreFloat4x4(&m_mWorldMatrix, WorldMatrix);
}

//void CTransform::Bind_ShaderResource(const std::string& strConstantName, CShader* pShader)
//{
//
//}

void CTransform::Refresh_WorldMatrix(TRANSFORM_ENUM eEnum)
{
	switch (eEnum)
	{
	case LOOK:
		{
		_vector vLook = XMLoadFloat3(&m_vLook);
		_vector vRight = XMVector3Cross(XMVectorSet(0.f, 1.f, 0.f, 0.f), vLook);
		_vector	vUp = XMVector3Cross(vLook, vRight);

		Vector3 ScaleLength = Get_ScaleXYZ(); // 기존 스케일 저장
		
		Set_WorldMatrix(MAT_RIGHT, vRight);
		Set_WorldMatrix(MAT_UP, vUp);
		Set_WorldMatrix(MAT_LOOK, vLook);

		Refresh_MatrixScaled(ScaleLength); // 변형된 스케일 정상화
		}
		break;
	case POSITION:
		memcpy(&m_mWorldMatrix.m[MAT_POSITION], &m_vPosition, sizeof(Vector3));
		break;
	case SCALE:
		Refresh_MatrixScaled(m_vScale);
		break;
	default: // Scale Look PositionVal 모두 업데이트
		{
		Refresh_MatrixScaled(m_vScale);

		_vector vLook = XMLoadFloat3(&m_vLook);
		_vector vRight = XMVector3Cross(XMVectorSet(0.f, 1.f, 0.f, 0.f), vLook);
		_vector	vUp = XMVector3Cross(vLook, vRight);

		Vector3 ScaleLength = Get_ScaleXYZ(); // 기존 스케일 저장

		Set_WorldMatrix(MAT_RIGHT, vRight);
		Set_WorldMatrix(MAT_UP, vUp);
		Set_WorldMatrix(MAT_LOOK, vLook);

		Refresh_MatrixScaled(ScaleLength); // 변형된 스케일 정상화
		Set_WorldMatrix(MAT_POSITION, XMLoadFloat3(&m_vPosition));
		}
		break;
	}
}
_vector CTransform::Get_MatrixRow(MATRIX_ENUM eEnum)
{
	_matrix WorldMatrix = XMLoadFloat4x4(&m_mWorldMatrix);

	return WorldMatrix.r[eEnum];
}

Vector3 CTransform::Get_ScaleXYZ()
{
	return Vector3(
		 XMVectorGetX(XMVector3Length(Get_MatrixRow(MAT_RIGHT))) ,
		 XMVectorGetX(XMVector3Length(Get_MatrixRow(MAT_UP))) ,
		 XMVectorGetX(XMVector3Length(Get_MatrixRow(MAT_LOOK)))
	);
}

void CTransform::Refresh_MatrixScaled(const Vector3& scale)
{
	Set_WorldMatrix(MAT_RIGHT, XMVector3Normalize(Get_MatrixRow(MAT_RIGHT) * scale.x));
	Set_WorldMatrix(MAT_UP, XMVector3Normalize(Get_MatrixRow(MAT_UP) * scale.y));
	Set_WorldMatrix(MAT_LOOK, XMVector3Normalize(Get_MatrixRow(MAT_LOOK) * scale.z));
}

void CTransform::Rotate(CTransform::AXIS_ENUM eAxis, FLOAT fRotSpeed)
{
	Matrix rotMat; // = Matrix::CreateRotationX()

	switch (eAxis)
	{
	case AXIS_X:
		rotMat = Matrix::CreateRotationX(fRotSpeed);
		break;
	case AXIS_Y:
		rotMat = Matrix::CreateRotationY(fRotSpeed);
		break;
	case AXIS_Z:
		rotMat = Matrix::CreateRotationZ(fRotSpeed);
		break;

	default:
		break;
	}

	m_mWorldMatrix *= rotMat;

	m_mWorldMatrix.Right().Normalize();
	m_mWorldMatrix.Up().Normalize();
	m_mWorldMatrix.Forward().Normalize();

	m_vRight = m_mWorldMatrix.Right();
	m_vUp = m_mWorldMatrix.Up();
	m_vLook = m_mWorldMatrix.Forward();

	//m_vLook = *(Vector3*)(&m_mWorldMatrix.m[LOOK]); // Update Look Var
}
