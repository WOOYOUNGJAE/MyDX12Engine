#pragma once
#include "GameObject.h"
#include "Client_Defines.h"

NAMESPACE_(Engine)
class CTransform;
class CMeshObject;
class CShader;
class CRenderer;
class CMaterial;
class CTextureCompo;
_NAMESPACE

NAMESPACE_(Client)

class CTreeBillboard : public CGameObject
{


protected:
	CTreeBillboard() = default;
	CTreeBillboard(CTreeBillboard& rhs) : CGameObject(rhs) {}
	~CTreeBillboard() override = default;

public:
	static CTreeBillboard* Create();
	CGameObject* Clone(UINT* pInOutRenderNumbering, void* pArg) override;
	HRESULT Initialize_Prototype() override;
	HRESULT Initialize(void* pArg) override;
	void Tick(_float fDeltaTime) override;
	void Late_Tick(_float fDeltaTime) override;
	void Render_Tick() override;
	void Render(ID3D12GraphicsCommandList* pCmdList, FrameResource* pFrameResource, UINT iRenderingElementIndex) override;
	HRESULT Free() override;
public: // getter setter
	D3D12_PRIMITIVE_TOPOLOGY PrimitiveType()const override {return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;}
	MATERIAL_INFO Get_MaterialInfo() override;
	CMaterial* Get_Material() override { return m_pMaterialCom; }
	// Transform
	Matrix Get_WorldMatrix() override;
	Vector3 Get_Pos() override;
	Vector3 Get_ScaleXYZ() override;
	void Set_Position(const Vector3& vPos) override;
	void Set_Scale(const Vector3& vScale) override;

private:
	CTransform* m_pTransformCom = nullptr;
	CMeshObject* m_pMeshObjectCom = nullptr;
	CShader* m_pShaderCom = nullptr;
	CRenderer* m_pRendererCom = nullptr;
	CMaterial* m_pMaterialCom = nullptr;
	CTextureCompo* m_pTextureCom = nullptr;
};

_NAMESPACE