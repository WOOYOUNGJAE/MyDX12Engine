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
	const Matrix& WorldMatrix() { return m_WorldMatrix; }
	XMMATRIX WorldMatrix_Inverse() { return XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_WorldMatrix)); }
	Vector3 Position() { return m_Position; }
	Vector3 Right() { return m_Right; }
	Vector3 Up() { return m_Up; }
	Vector3 Look() { return m_Look; }
	void Set_Look(const Vector3& look) { m_Look = look; Refresh_WorldMatrix(LOOK); }
	void Set_Scale(const Vector3& scale) { m_Scale = scale; Refresh_WorldMatrix(SCALE);	}
	void Set_Position(const Vector3& pos) { m_Position = pos; Refresh_WorldMatrix(POSITION); }
	void Set_WorldMatrix(const Matrix& matrix) { m_WorldMatrix = matrix; }
	void Set_WorldMatrix(MATRIX_ENUM eEnum, _fvector vVec);

public:
	_vector Get_MatrixRow(MATRIX_ENUM eEnum);
	Vector3 Get_ScaleXYZ();
	void Refresh_WorldMatrix(TRANSFORM_ENUM eEnum = TRANSFORM_ENUM_END);
	void Refresh_MatrixScaled(const Vector3& scale);

public:
	Matrix m_WorldMatrix;
	Vector3 m_Scale;
	Vector3 m_Right;
	Vector3 m_Up;
	Vector3 m_Look;
	Vector3 m_Position;

};

_NAMESPACE
