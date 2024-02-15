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
	~CTransform() override = default;

public:
	static CTransform* Create();
	CComponent* Clone(void* pArg) override;
	HRESULT Initialize_Prototype() override;
	HRESULT Initialize(void* pArg) override;
	HRESULT Free() override;
public: // getter setter
	const XMFLOAT4X4& WorldMatrix() { return m_WorldMatrix; }
	XMMATRIX WorldMatrix_Inverse() { return XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_WorldMatrix)); }
	XMFLOAT3 Position() { return m_Position; }
	XMFLOAT3 Right() { return m_Right; }
	XMFLOAT3 Up() { return m_Up; }
	XMFLOAT3 Look() { return m_Look; }
	void Set_Look(const XMFLOAT3& look) { m_Look = look; Refresh_WorldMatrix(LOOK); }
	void Set_Scale(const XMFLOAT3& scale) { m_Scale = scale; Refresh_WorldMatrix(SCALE);	}
	void Set_Position(const XMFLOAT3& pos) { m_Position = pos; Refresh_WorldMatrix(POSITION); }
	void Set_WorldMatrix(const XMFLOAT4X4& matrix) { m_WorldMatrix = matrix; }
	void Set_WorldMatrix(MATRIX_ENUM eEnum, _fvector vVec);
public:
	//void Bind_ShaderResource(const std::string& strConstantName, class CShader* pShader);

private:
	_vector Get_MatrixRow(MATRIX_ENUM eEnum);
	XMFLOAT3 Get_ScaleXYZ();
	void Refresh_WorldMatrix(TRANSFORM_ENUM eEnum = TRANSFORM_ENUM_END);
	void Refresh_MatrixScaled(const XMFLOAT3& scale);

private:
	XMFLOAT4X4 m_WorldMatrix;
	XMFLOAT3 m_Scale;
	XMFLOAT3 m_Right;
	XMFLOAT3 m_Up;
	XMFLOAT3 m_Look;
	XMFLOAT3 m_Position;

};

_NAMESPACE
