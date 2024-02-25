#include "Cube.h"
#include "MeshObject.h"
#include "Transform.h"
#include "Renderer.h"
#include "Shader.h"
#include "TextureCompo.h"
#include "Material.h"
#include "MeshData.h"

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

	std::vector<wstring> strMeshTagArr = { L"CubeMesh", };
	hr = Add_Component(L"MeshObject", reinterpret_cast<CComponent**>(&m_pMeshObjectCom), &strMeshTagArr);
	if (FAILED(hr)) return hr;
	hr = Add_Component(L"Renderer", reinterpret_cast<CComponent**>(&m_pRendererCom));
	if (FAILED(hr)) return hr;
	hr = Add_Component(L"Shader_Simple2", reinterpret_cast<CComponent**>(&m_pShaderCom));
	if (FAILED(hr)) return hr;
	hr = Add_Component(L"Texture", reinterpret_cast<CComponent**>(&m_pTextureCom), &wstring(L"Texture_NightSkybox"));
	//hr = Add_Component(L"Texture", reinterpret_cast<CComponent**>(&m_pTextureCom), &wstring(L"Texture_ice"));
	if (FAILED(hr)) return hr;

	MATERIAL_INFO matInfo{ Vector3::Zero * 0.5f, 0.5f, Vector3::One * 0.5f, 0.f, Vector3::One * 0.5f};
	hr = Add_Component(L"Material", reinterpret_cast<CComponent**>(&m_pMaterialCom), &matInfo);


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
	m_pRendererCom->AddTo_RenderGroup(RENDER_AFTER, NOBLEND, SHADERTYPE_SIMPLE3, ROOTSIG_DEFAULT, this);
}

void CCube::Render(ID3D12GraphicsCommandList* pCmdList, FrameResource* pFrameResource)
{
	// Update CB
	OBJECT_CB objConstants;
	objConstants.mWorldMat = Get_WorldMatrix().Transpose();
	objConstants.mInvTranspose = Get_WorldMatrix().Invert(); // Transpose µÎ¹ø
	objConstants.material = Get_MaterialInfo();
	pFrameResource->pObjectCB->CopyData(m_iClonedNum - 1, objConstants);

	pCmdList->IASetPrimitiveTopology(PrimitiveType());

	// for each meshes
	for (UINT i = 0; i < m_pMeshObjectCom->Get_vecMeshData()->size(); ++i)
	{
		CMeshData* pMesh = (*m_pMeshObjectCom->Get_vecMeshData())[i];

		pCmdList->IASetVertexBuffers(0, 1, pMesh->Get_VertexBufferViewPtr());
		pCmdList->IASetIndexBuffer(pMesh->Get_IndexBufferViewPtr());

		// Set Descriptor Tables
		pCmdList->SetGraphicsRootDescriptorTable(0, m_pRendererCom->Get_HandleOffsettedGPU(Get_CbvSrvUavHeapOffset_Texture()));
		pCmdList->SetGraphicsRootDescriptorTable(1, m_pRendererCom->Get_ObjCbvHandleOffsettedGPU());

		//pCmdList->DrawInstanced(24, 1, 0, 0);
		pCmdList->DrawIndexedInstanced(
			36,
			1,
			0,
			0,
			0);
	}
}



HRESULT CCube::Free()
{
	Safe_Release(m_pMaterialCom);
	Safe_Release(m_pTextureCom);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pRendererCom);
	Safe_Release(m_pMeshObjectCom);
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
	return D3D12_VERTEX_BUFFER_VIEW();
}

D3D12_INDEX_BUFFER_VIEW CCube::IndexBufferView() const
{
	return D3D12_INDEX_BUFFER_VIEW();
}

_uint CCube::Num_Indices()
{
	return 0;
	//return m_pMeshObjectCom->Num_Indices();
}

MATERIAL_INFO CCube::Get_MaterialInfo()
{
	return m_pMaterialCom->Get_MaterialInfo();
}
