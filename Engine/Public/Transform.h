#pragma once
#include "Component.h"

NAMESPACE_(Engine)

// Client에게 TransformOperation (Look, Position, Scale) 클래스를 통해 접근 허용,
// TransformOperation을 통해 정보가 바뀌면 자동으로 CTransform의 Refresh를 통해
// CTransform의 멤버변수를 바꾸고 그에 맞게 월드매트릭스도 업데이트
// XMVECTOR - XMFLOAT 주의
class ENGINE_DLL CTransform final : public CComponent
{
public: // typedef
	enum TRANSFORM_ENUM {LOOK, POSITION, SCALE, TRANSFORM_ENUM_END,};
	enum MATRIX_ENUM { MAT_RIGHT, MAT_UP, MAT_LOOK, MAT_POSITION, MATRIX_ENUM_END };
private:
	CTransform() = default;
	CTransform(CTransform& rhs) = default;
	~CTransform() override = default;

public:
	CComponent* Clone(void* pArg) override;

public: // getter setter
	_float4x4 WorldMatrix() { return m_WorldMatrix; }
	_float3 Position() { return m_Position; }
	_float3 Right() { return m_Right; }
	_float3 Up() { return m_Up; }
	_float3 Look() { return m_Look; }
	void Set_Look(const _float3& look) { m_Look = look; }
	void Set_Scale(const _float3& scale) { m_Scale = scale; }
	void Set_Position(const _float3& pos) { m_Position = pos; }
	void Set_WorldMatrix(const _float4x4& matrix) { m_WorldMatrix = matrix; }
	void Set_WorldMatrix(MATRIX_ENUM eEnum, _fvector vVec);
public:
	void Refresh_Transform(TRANSFORM_ENUM eEnum = TRANSFORM_ENUM_END);
private:
	_vector Get_MatrixRow(MATRIX_ENUM eEnum);
	_float3 Get_ScaleXYZ();
	void Refresh_MatrixScaled(const _float3& scale);
public:
	_float3 m_Position;

private:
	_float4x4 m_WorldMatrix;
	_float3 m_Scale;
	_float3 m_Right;
	_float3 m_Up;
	_float3 m_Look;

};

#pragma region TransformOperation
// 외부에서 직접적으로 포지션 값을 변경하는 동시에 트랜스폼의 월드 매트릭스도 변경하게 끔

class Position
{
public:
	Position() = default;
	~Position()
	{	
		Safe_Release(m_pOwner);
	}

public: // Operator Overload
	_float3 operator+(_vector vVector)
	{
		_vector vPos = XMLoadFloat3(&m_Position);

		vPos = XMVectorAdd(vPos, vVector);
		XMStoreFloat3(&m_Position, vPos);
		m_pOwner->Set_Position(m_Position);
		m_pOwner->Refresh_Transform();
		return m_Position;
	}
	_float3 operator-(_vector vVector)
	{
		_vector vPos = XMLoadFloat3(&m_Position);

		vPos = XMVectorAdd(vPos, -vVector);
		XMStoreFloat3(&m_Position, vPos);
		return m_Position;
	}
public:
	void Set_Owner(CTransform* pOwner) { m_pOwner = pOwner; Safe_AddRef(m_pOwner); }
private:
	_float3 m_Position;
	CTransform* m_pOwner = nullptr;
};
#pragma endregion TransformOperation

_NAMESPACE