#pragma once
#include "Component.h"

NAMESPACE_(Engine)

// XMVECTOR - XMFLOAT ¡÷¿«
class ENGINE_DLL CTransform final : public CComponent
{
public: // typedef
	enum TRANSFORM_ENUM {LOOK, POSITION, SCALE, TRANSFORM_ENUM_END,};
	enum MATRIX_ENUM { MAT_RIGHT, MAT_UP, MAT_LOOK, MAT_POSITION, MATRIX_ENUM_END };
private:
	CTransform() = default;
	CTransform(CTransform& rhs) : CComponent(rhs){}
	~CTransform() override = default;

public:
	static CTransform* Create();
	CComponent* Clone(void* pArg) override;
	HRESULT Initialize_Prototype() override;
	HRESULT Initialize(void* pArg) override;
	HRESULT Free() override;
public: // getter setter
	const Matrix& WorldMatrix() { return m_mWorldMatrix; }
	XMMATRIX WorldMatrix_Inverse() { return XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_mWorldMatrix)); }
	Vector3 Position() { return m_vPosition; }
	Vector3 Right() { return m_vRight; }
	Vector3 Up() { return m_vUp; }
	Vector3 Look() { return m_vLook; }
	void Set_Look(const Vector3& look) { m_vLook = look; Refresh_WorldMatrix(LOOK); }
	void Set_Scale(const Vector3& scale) { m_vScale = scale; Refresh_WorldMatrix(SCALE);	}
	void Set_Position(const Vector3& pos) { m_vPosition = pos; Refresh_WorldMatrix(POSITION); }
	void Set_WorldMatrix(const Matrix& matrix) { m_mWorldMatrix = matrix; }
	void Set_WorldMatrix(MATRIX_ENUM eEnum, _fvector vVec);

public:
	_vector Get_MatrixRow(MATRIX_ENUM eEnum);
	Vector3 Get_ScaleXYZ();
	void Refresh_WorldMatrix(TRANSFORM_ENUM eEnum = TRANSFORM_ENUM_END);
	void Refresh_MatrixScaled(const Vector3& scale);

public:
	Matrix m_mWorldMatrix;
	Vector3 m_vScale;
	Vector3 m_vRight;
	Vector3 m_vUp;
	Vector3 m_vLook;
	Vector3 m_vPosition;

};

_NAMESPACE
