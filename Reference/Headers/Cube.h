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
	Matrix Get_WorldMatrix() override;
	Vector3 Get_Pos() override;
	D3D12_VERTEX_BUFFER_VIEW VertexBufferView() override;
	D3D12_INDEX_BUFFER_VIEW IndexBufferView() const override;
	_uint Num_Indices() override;

protected:
	class CTransform* m_pTransformCom = nullptr;
	class CCubeMesh* m_pCubeMeshCom = nullptr;
	class CShader* m_pShaderCom = nullptr;
	class CRenderer* m_pRendererCom = nullptr;
};

_NAMESPACE