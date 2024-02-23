#pragma once
#include "GameObject.h"

NAMESPACE_(Engine)

class ENGINE_DLL CCube : public CGameObject
{
protected:
	CCube() = default;
	CCube(CCube& rhs): CGameObject(rhs){}
	~CCube() override = default;

public: // LifeCycle
	static CCube* Create();
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
	MATERIAL_INFO Get_MaterialInfo() override;
	// Transform
	Matrix Get_WorldMatrix() override;
	Vector3 Get_Pos() override;
	Vector3 Get_ScaleXYZ() override;
	void Set_Position(const Vector3& vPos) override;
	void Set_Scale(const Vector3& vScale) override;
protected:
	class CTransform* m_pTransformCom = nullptr;
	//class CTriangleMesh* m_pTriangleMeshCom = nullptr;
	class CCubeMesh* m_pCubeMeshCom = nullptr;
	class CShader* m_pShaderCom = nullptr;
	class CRenderer* m_pRendererCom = nullptr;
	class CTextureCompo* m_pTextureCom = nullptr;
	class CMaterial* m_pMaterialCom = nullptr;
};

_NAMESPACE