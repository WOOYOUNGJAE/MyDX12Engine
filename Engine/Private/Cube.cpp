#include "Cube.h"
#include "CubeMesh.h"
#include "Transform.h"
#include "Renderer.h"
#include "Shader.h"
#include "TextureCompo.h"

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
	/*hr = Add_Component(L"TriangleMesh", reinterpret_cast<CComponent**>(&m_pTriangleMeshCom));
	if (FAILED(hr)) return hr;*/
	hr = Add_Component(L"CubeMesh", reinterpret_cast<CComponent**>(&m_pCubeMeshCom));
	if (FAILED(hr)) return hr;
	hr = Add_Component(L"Renderer", reinterpret_cast<CComponent**>(&m_pRendererCom));
	if (FAILED(hr)) return hr;
	hr = Add_Component(L"Shader_Simple2", reinterpret_cast<CComponent**>(&m_pShaderCom));
	if (FAILED(hr)) return hr;
	hr = Add_Component(L"Texture", reinterpret_cast<CComponent**>(&m_pTextureCom), &wstring(L"Texture_Checkboard"));
	//hr = Add_Component(L"Texture", reinterpret_cast<CComponent**>(&m_pTextureCom), &wstring(L"Texture_ice"));
	if (FAILED(hr)) return hr;

	m_iCbvSrvUavOffset = m_pTextureCom->m_iCbvSrvUavHeapOffset;

	m_pTransformCom->Set_Position(static_cast<GAMEOBJECT_INIT_DESC*>(pArg)->vStartPos);

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

void CCube::Render_Tick()
{
	int a = 0;
	m_pRendererCom->AddTo_RenderGroup(RENDER_AFTER, NOBLEND, SHADERTYPE_SIMPLE2, ROOTSIG_DEFAULT, this);
}


HRESULT CCube::Free()
{
	Safe_Release(m_pTextureCom);
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

Matrix CCube::Get_WorldMatrix()
{
	return m_pTransformCom->WorldMatrix();
}

Vector3 CCube::Get_Pos()
{
	return m_pTransformCom->Position();
}

Vector3 CCube::Get_ScaleXYZ()
{
	return CGameObject::Get_ScaleXYZ();
}

void CCube::Set_Position(const Vector3& vPos)
{
	CGameObject::Set_Position(vPos);
}

void CCube::Set_Scale(const Vector3& vScale)
{
	CGameObject::Set_Scale(vScale);
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
