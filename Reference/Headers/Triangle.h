#pragma once
#include "GameObject.h"

NAMESPACE_(Engine)

class CTriangle : public CGameObject
{
protected:
	CTriangle() = default;
	CTriangle(CTriangle& rhs) : CGameObject(rhs) {}
	~CTriangle() override = default;

public: // LifeCycle
	static CTriangle* Create();
	CGameObject* Clone(void* pArg) override;
	HRESULT Initialize_Prototype() override;
	HRESULT Initialize(void* pArg) override;
	void Tick(_float fDeltaTime) override;
	void Late_Tick(_float fDeltaTime) override;
	void Render_Tick() override;
	HRESULT Free() override;
public: // getter setter 
	int& Get_NumFrameDirtyRef() override;
	D3D12_VERTEX_BUFFER_VIEW VertexBufferView() override;
	D3D12_INDEX_BUFFER_VIEW IndexBufferView() const override;
	_uint Num_Indices() override;
	Matrix Get_WorldMatrix() override;
	Vector3 Get_Pos() override;
	Vector3 Get_ScaleXYZ() override;
	void Set_Position(const Vector3& vPos) override;
	void Set_Scale(const Vector3& vScale) override;
protected:
	class CTransform* m_pTransformCom = nullptr;
	//class CTriangleMesh* m_pTriangleMeshCom = nullptr;
	class CTriangleMesh_PT* m_pTriangleMeshCom = nullptr;
	class CShader* m_pShaderCom = nullptr;
	class CRenderer* m_pRendererCom = nullptr;
	class CTextureCompo* m_pTextureCom = nullptr;
};

_NAMESPACE