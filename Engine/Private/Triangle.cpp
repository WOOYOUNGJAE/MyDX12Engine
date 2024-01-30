#include "Triangle.h"
#include "TriangleMesh.h"
#include "Transform.h"
#include "Renderer.h"
#include "Shader.h"

CTriangle* CTriangle::Create()
{
	CTriangle* pInstance = new CTriangle;

	if (pInstance)
	{
		pInstance->Initialize_Prototype();
	}

	return pInstance;
}

CGameObject* CTriangle::Clone(void* pArg)
{
	CTriangle* pInstance = new CTriangle(*this);

	if (pInstance)
	{
		if (FAILED(pInstance->Initialize(pArg)))
		{
			Safe_Release(pInstance);
			return pInstance;
		}
	}
	pInstance->m_bIsCloned = true;
	return pInstance;
}

HRESULT CTriangle::Initialize_Prototype()
{

	return S_OK;
}

HRESULT CTriangle::Initialize(void* pArg)
{
	HRESULT hr = S_OK;
	// ±âº» ÄÄÆ÷³ÍÆ® ºÎÂø
	hr = Add_Component(L"Transform", reinterpret_cast<CComponent**>(&m_pTransformCom));
	if (FAILED(hr)) return hr;
	hr = Add_Component(L"TriangleMesh", reinterpret_cast<CComponent**>(&m_pTriangleMeshCom));
	if (FAILED(hr)) return hr;
	hr = Add_Component(L"Renderer", reinterpret_cast<CComponent**>(&m_pRendererCom));
	if (FAILED(hr)) return hr;
	hr = Add_Component(L"Shader_Simple", reinterpret_cast<CComponent**>(&m_pShaderCom));
	if (FAILED(hr)) return hr;

	return hr;
}

void CTriangle::Tick(_float fDeltaTime)
{
	int a = 0;
}

void CTriangle::Late_Tick(_float fDeltaTime)
{
	int a = 0;
}

void CTriangle::Render_Tick()
{	
	m_pRendererCom->AddTo_RenderGroup(RENDER_AFTER, NOBLEND, SHADERTYPE_SIMPLE, PARAM_SIMPLE, this);
}

HRESULT CTriangle::Free()
{
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pRendererCom);
	Safe_Release(m_pTriangleMeshCom);
	Safe_Release(m_pTransformCom);

	return CGameObject::Free();
}

int& CTriangle::Get_NumFrameDirtyRef()
{
	return m_pShaderCom->Get_NumDirtyRef();
}

_matrix CTriangle::Get_WorldMatrix()
{
	return m_pTransformCom->WorldMatrix();
}

_float3 CTriangle::Get_Pos()
{
	return m_pTransformCom->Position();
}

D3D12_VERTEX_BUFFER_VIEW CTriangle::VertexBufferView()
{
	return m_pTriangleMeshCom->VertexBufferView();
}

D3D12_INDEX_BUFFER_VIEW CTriangle::IndexBufferView() const
{
	return m_pTriangleMeshCom->IndexBufferView();
}

_uint CTriangle::Num_Indices()
{
	return 0;
}
