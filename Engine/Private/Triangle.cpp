#include "Triangle.h"
#include "TriangleMesh.h"
#include "TriangleMesh_PT.h"
#include "Transform.h"
#include "Renderer.h"
#include "Shader.h"
#include "TextureCompo.h"

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
	/*hr = Add_Component(L"TriangleMesh", reinterpret_cast<CComponent**>(&m_pTriangleMeshCom));
	if (FAILED(hr)) return hr;*/
	hr = Add_Component(L"TriangleMesh_PT", reinterpret_cast<CComponent**>(&m_pTriangleMeshCom));
	if (FAILED(hr)) return hr;
	hr = Add_Component(L"Renderer", reinterpret_cast<CComponent**>(&m_pRendererCom));
	if (FAILED(hr)) return hr;
	hr = Add_Component(L"Shader_Simple", reinterpret_cast<CComponent**>(&m_pShaderCom));
	if (FAILED(hr)) return hr;
	hr = Add_Component(L"Texture", reinterpret_cast<CComponent**>(&m_pTextureCom), &wstring(L"Texture_Checkboard"));
	//hr = Add_Component(L"Texture", reinterpret_cast<CComponent**>(&m_pTextureCom), &wstring(L"Texture_ice"));
	if (FAILED(hr)) return hr;

	m_iCbvSrvUavOffset = m_pTextureCom->m_iCbvSrvUavHeapOffset;

	return hr;
}

void CTriangle::Tick(_float fDeltaTime)
{
	m_pTransformCom->m_Position += Vector3(0.001f, 0.f, 0.f);
	auto r = m_pTransformCom->Position();
	m_pTransformCom->Refresh_WorldMatrix(CTransform::POSITION);
}

void CTriangle::Late_Tick(_float fDeltaTime)
{
	int a = 0;
}

void CTriangle::Render_Tick()
{
	// Bind Resource


	//m_pRendererCom->AddTo_RenderGroup(RENDER_AFTER, NOBLEND, SHADERTYPE_SIMPLE, ROOTSIG_DEFAULT, this);
	m_pRendererCom->AddTo_RenderGroup(RENDER_AFTER, NOBLEND, SHADERTYPE_SIMPLE2, ROOTSIG_DEFAULT, this);
}

HRESULT CTriangle::Free()
{
	Safe_Release(m_pTextureCom);
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

XMFLOAT4X4 CTriangle::Get_WorldMatrix()
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
