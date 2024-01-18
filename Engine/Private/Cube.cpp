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
	CCube* pInstance = new CCube(*this);

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

HRESULT CCube::Initialize_Prototype()
{

	return S_OK;
}

HRESULT CCube::Initialize(void* pArg)
{
	HRESULT hr = S_OK;
	// ±âº» ÄÄÆ÷³ÍÆ® ºÎÂø
	hr = Add_Component(L"Transform", reinterpret_cast<CComponent**>(&m_pTransformCom));
	if (FAILED(hr)) return hr;
	hr = Add_Component(L"CubeMesh", reinterpret_cast<CComponent**>(&m_pCubeMeshCom));
	if (FAILED(hr)) return hr;
	hr = Add_Component(L"Renderer", reinterpret_cast<CComponent**>(&m_pRendererCom));
	if (FAILED(hr)) return hr;
	hr = Add_Component(L"Shader_Default", reinterpret_cast<CComponent**>(&m_pShaderCom));
	if (FAILED(hr)) return hr;

	return hr;
}

void CCube::Tick(_float fDeltaTime)
{
	int a = 0;
}

void CCube::Late_Tick(_float fDeltaTime)
{
	int a = 0;
}

void CCube::Pipeline_Tick()
{
	int a = 0;

}

void CCube::Render()
{
	int a = 0;
	m_pRendererCom->AddTo_RenderGroup(CRenderer::NOBLEND, this);
}

HRESULT CCube::Free()
{
	Safe_Release(m_pShaderCom);
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

D3D12_VERTEX_BUFFER_VIEW CCube::VertexBufferView()
{
	return m_pCubeMeshCom->VertexBufferView();
}

D3D12_INDEX_BUFFER_VIEW CCube::IndexBufferView() const
{
	return m_pCubeMeshCom->IndexBufferView();
}

_uint CCube::Num_Indices()
{
	return m_pCubeMeshCom->Num_Indices();
}
