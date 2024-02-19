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

void CTransform::Set_WorldMatrix(MATRIX_ENUM eEnum, const Vector4& vVec)
{
	*(Vector4*)m_mWorldMatrix.m[eEnum] = vVec;
}

void CTransform::Set_WorldMatrix(MATRIX_ENUM eEnum, const Vector3& vVec)
{
	FLOAT w = eEnum == MAT_POSITION ? 1.f : 0.f;

	memcpy(&m_mWorldMatrix.m[eEnum], &Vector4(vVec.x, vVec.y, vVec.z, w), sizeof(Vector4));
	//*(Vector4*)m_mWorldMatrix.m[eEnum] = Vector4(vVec.x, vVec.y, vVec.z, w);
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
		Vector3 ScaleLength = Get_ScaleXYZ(); // 기존 스케일 저장

		m_vRight = Vector3::Up.Cross(m_vLook);
		m_vUp = m_vLook.Cross(m_vRight);

		Set_WorldMatrix(MAT_RIGHT, m_vRight);
		Set_WorldMatrix(MAT_UP, m_vUp);
		Set_WorldMatrix(MAT_LOOK, m_vLook);

		Refresh_MatrixScaled(ScaleLength); // 변형된 스케일 정상화

		//_vector vLook = XMLoadFloat3(&m_vLook);
		//_vector vRight = XMVector3Cross(XMVectorSet(0.f, 1.f, 0.f, 0.f), vLook);
		//_vector	vUp = XMVector3Cross(vLook, vRight);

		//Vector3 ScaleLength = Get_ScaleXYZ(); // 기존 스케일 저장
		//
		//Set_WorldMatrix(MAT_RIGHT, vRight);
		//Set_WorldMatrix(MAT_UP, vUp);
		//Set_WorldMatrix(MAT_LOOK, vLook);

		//Refresh_MatrixScaled(ScaleLength); // 변형된 스케일 정상화
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

		Vector3 ScaleLength = Get_ScaleXYZ(); // 기존 스케일 저장
		m_vRight = Vector3::Up.Cross(m_vLook);
		m_vUp = m_vLook.Cross(m_vRight);

		Set_WorldMatrix(MAT_RIGHT, m_vRight);
		Set_WorldMatrix(MAT_UP, m_vUp);
		Set_WorldMatrix(MAT_LOOK, m_vLook);

		Refresh_MatrixScaled(ScaleLength); // 변형된 스케일 정상화

		memcpy(&m_mWorldMatrix.m[MAT_POSITION], &m_vPosition, sizeof(Vector3));
		}
		break;
	}
}

Vector3 CTransform::Get_ScaleXYZ()
{
	return Vector3(
		 m_mWorldMatrix.Right().Length(),
		m_mWorldMatrix.Up().Length(),
		m_mWorldMatrix.Forward().Length()
	);
}

void CTransform::Refresh_MatrixScaled(const Vector3& scale)
{
	Set_WorldMatrix(MAT_RIGHT, (Vector3)XMVector3Normalize(m_mWorldMatrix.Right()) * scale.x);
	Set_WorldMatrix(MAT_UP, (Vector3)XMVector3Normalize(m_mWorldMatrix.Up()) * scale.y);
	Set_WorldMatrix(MAT_LOOK, (Vector3)XMVector3Normalize(m_mWorldMatrix.Forward()) * scale.z);
}

void CTransform::Rotate(Vector3 vAxis, FLOAT fRotSpeed)
{
	Matrix rotMat; // = Matrix::CreateRotationX()

	rotMat = XMMatrixRotationAxis(XMLoadFloat3(&vAxis), fRotSpeed);

	m_vRight.TransformNormal(m_vRight, rotMat, m_vRight);
	m_vUp.TransformNormal(m_vUp, rotMat, m_vUp);
	m_vLook.TransformNormal(m_vLook, rotMat, m_vLook);

	Set_WorldMatrix(MAT_RIGHT, Vector4(m_vRight.x, m_vRight.y, m_vRight.z, 0) );
	Set_WorldMatrix(MAT_UP, Vector4(m_vUp.x, m_vUp.y, m_vUp.z, 0) );
	Set_WorldMatrix(MAT_LOOK, Vector4(m_vLook.x, m_vLook.y, m_vLook.z, 0) );
}

void CTransform::Move_Forward(FLOAT fRotSpeed)
{
	m_vPosition = m_vPosition + m_vLook * fRotSpeed;
	Refresh_WorldMatrix(POSITION);
}

void CTransform::Move_Right(FLOAT fRotSpeed)
{
	m_vPosition = m_vPosition + m_vRight * fRotSpeed;
	Refresh_WorldMatrix(POSITION);
}

void CTransform::Move_Up(FLOAT fRotSpeed)
{
	m_vPosition = m_vPosition + m_vUp * fRotSpeed;
	Refresh_WorldMatrix(POSITION);
}
