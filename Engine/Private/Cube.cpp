#include "CubeMesh.h"
#include "Transform.h"
#include "Cube.h"
#include "Renderer.h"
#include "Shader.h"

CCube* CCube::Create()
{
	CCube* pInstance = new CCube;

	if (pInstance)
	{
		pInstance->Initialize_Prototype();
	}

	return pInstance;
}

CGameObject* CCube::Clone(void* pArg)
{
	return nullptr;
}

HRESULT CCube::Initialize_Prototype()
{
	// ±âº» ÄÄÆ÷³ÍÆ® ºÎÂø
	Add_Component(L"Transform", reinterpret_cast<CComponent**>(&m_pTransformCom));
	Add_Component(L"CubeMesh", reinterpret_cast<CComponent**>(&m_pCubeMeshCom));
	Add_Component(L"Renderer", reinterpret_cast<CComponent**>(&m_pRendererCom));

	return S_OK;
}

HRESULT CCube::Initialize(void* pArg)
{
	return S_OK;
}

void CCube::Tick(_float fDeltaTime)
{
}

void CCube::Late_Tick(_float fDeltaTime)
{
}

void CCube::Pipeline_Tick()
{

}

void CCube::Render()
{
	m_pRendererCom->AddTo_RenderGroup(CRenderer::NOBLEND, this);
}

HRESULT CCube::Free()
{
	Safe_Release(m_pRendererCom);
	Safe_Release(m_pCubeMeshCom);
	Safe_Release(m_pTransformCom);

	return CGameObject::Free();
}

int& CCube::Get_NumFrameDirtyRef()
{
	return m_pShaderCom->Get_NumDirtyRef();
}

_matrix CCube::Get_WorldMatrix()
{
	return m_pTransformCom->WorldMatrix();
}

_float3 CCube::Get_Pos()
{
	return m_pTransformCom->Position();
}
