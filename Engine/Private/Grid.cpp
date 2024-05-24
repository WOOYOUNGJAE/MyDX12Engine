#include "Grid.h"
#include "TextureCompo.h"
#include "MeshObject.h"
#include "Transform.h"
#include "Renderer.h"
#include "Shader.h"
#include "Material.h"
#include "MeshData.h"
#include "FrameResource.h"
CGrid::CGrid(CGrid& rhs) : CGameObject(rhs),
m_strGridMeshTag(rhs.m_strGridMeshTag)
{
}

CGrid* CGrid::Create(GEOMETRY_TYPE eGeometryType)
{
	CGrid* pInstance = new CGrid;

	//pInstance->m_strGridMeshTag = eGeometryType;
	pInstance->m_eGeometryType = eGeometryType;

	pInstance->m_bIsPrototype = true;
	if (pInstance)
	{
		pInstance->Initialize_Prototype();
	}

	return pInstance;
}

CGameObject* CGrid::Clone(UINT* pInOutRenderNumbering, void* pArg)
{
	CGrid* pInstance = new CGrid(*this);

	if (pInstance)
	{
		++(*pInOutRenderNumbering);
		pInstance->m_iRenderNumbering_ZeroIfNotRendered = *pInOutRenderNumbering;
		if (FAILED(pInstance->Initialize(pArg)))
		{
			Safe_Release(pInstance);
			return pInstance;
		}
	}
	return pInstance;
}

HRESULT CGrid::Initialize_Prototype()
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT CGrid::Initialize(void* pArg)
{
	HRESULT hr = S_OK;
	// ±âº» ÄÄÆ÷³ÍÆ® ºÎÂø
	hr = Add_Component(L"Transform", reinterpret_cast<CComponent**>(&m_pTransformCom));
	if (FAILED(hr)) return hr;
	/*hr = Add_Component(L"TriangleMesh", reinterpret_cast<CComponent**>(&m_pTriangleMeshCom));
	if (FAILED(hr)) return hr;*/
	MESHOBJ_INIT_DESC meshObjDesc{ true, (UINT)m_eGeometryType, std::wstring()};
	hr = Add_Component(L"MeshObject", reinterpret_cast<CComponent**>(&m_pMeshObjectCom), &meshObjDesc);
	if (FAILED(hr)) return hr;


	hr = Add_Component(L"Renderer", reinterpret_cast<CComponent**>(&m_pRendererCom));
	if (FAILED(hr)) return hr;
	hr = Add_Component(L"Shader_Simple2", reinterpret_cast<CComponent**>(&m_pShaderCom));
	if (FAILED(hr)) return hr;
	hr = Add_Component(L"Texture", reinterpret_cast<CComponent**>(&m_pTextureCom), &wstring(L"Texture_ice"));
	if (FAILED(hr)) return hr; 

	MATERIAL_INFO matInfo{ Vector3::Zero * 0.5f, 0.5f, Vector3::One * 0.5f, 0.f, Vector3::One * 0.5f };
	hr = Add_Component(L"Material", reinterpret_cast<CComponent**>(&m_pMaterialCom), &matInfo);


	m_iTextureSrvOffset = m_pTextureCom->m_iCbvSrvUavHeapOffset;

	m_pTransformCom->Set_Position(static_cast<GAMEOBJECT_INIT_DESC*>(pArg)->vStartPos);
	m_pTransformCom->Set_Scale(static_cast<GAMEOBJECT_INIT_DESC*>(pArg)->vStartScale);

	return hr;
}

void CGrid::Tick(_float fDeltaTime)
{
	int a = 0;
}

void CGrid::Late_Tick(_float fDeltaTime)
{
	int a = 0;
}

void CGrid::Render_Tick()
{
	int a = 0;
	m_pRendererCom->AddTo_RenderGroup( RENDER_AFTER, RENDER_CULLMODE::NONE, NOBLEND, SHADERTYPE_SIMPLE3, ROOTSIG_DEFAULT, this);
}

void CGrid::Render(ID3D12GraphicsCommandList* pCmdList, FrameResource* pFrameResource, UINT iRenderingElementIndex)
{
	// Update CB
	OBJECT_CB objConstants;
	objConstants.mWorldMat = Get_WorldMatrix().Transpose();
	objConstants.mInvTranspose = Get_WorldMatrix().Invert(); // Transpose µÎ¹ø
	objConstants.material = Get_MaterialInfo();
	pFrameResource->pObjectCB->CopyData( iRenderingElementIndex, objConstants);

	pCmdList->IASetPrimitiveTopology(PrimitiveType());

	// for each meshes
	for (UINT i = 0; i < m_pMeshObjectCom->Get_vecMeshData()->size(); ++i)
	{
		CMeshData* pMesh = (*m_pMeshObjectCom->Get_vecMeshData())[i];

		UINT objCBByteSize = MyUtils::Align256(sizeof(OBJECT_CB));
		D3D12_GPU_VIRTUAL_ADDRESS objCBAddress =
			pFrameResource->pObjectCB->Get_UploadBuffer()->GetGPUVirtualAddress() + iRenderingElementIndex * objCBByteSize;

		pCmdList->IASetVertexBuffers(0, 1, pMesh->Get_VertexBufferViewPtr());
		pCmdList->IASetIndexBuffer(pMesh->Get_IndexBufferViewPtr());

		// Set Descriptor Tables
		pCmdList->SetGraphicsRootDescriptorTable(0, m_pRendererCom->Get_HandleOffsettedGPU(Get_CbvSrvUavHeapOffset_Texture()));
		pCmdList->SetGraphicsRootConstantBufferView(1, objCBAddress);
		/*pCmdList->SetGraphicsRootDescriptorTable(1,
			m_pRendererCom->Get_ObjCbvHandleOffsettedGPU(
				(m_iClonedNum - 1) * m_pRendererCom->Get_ObjCbvDescriptorSize()
			));*/

		pCmdList->DrawIndexedInstanced(
			pMesh->Num_Indices(),
			1,
			0,
			0,
			0);
	}
}



HRESULT CGrid::Free()
{
	Safe_Release(m_pMaterialCom);
	Safe_Release(m_pTextureCom);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pRendererCom);
	Safe_Release(m_pMeshObjectCom);
	Safe_Release(m_pTransformCom);

	return CGameObject::Free();
}

Matrix CGrid::Get_WorldMatrix()
{
	return m_pTransformCom->WorldMatrix();
}

Vector3 CGrid::Get_Pos()
{
	return m_pTransformCom->Position();
}

Vector3 CGrid::Get_ScaleXYZ()
{
	return CGameObject::Get_ScaleXYZ();
}

void CGrid::Set_Position(const Vector3& vPos)
{
	m_pTransformCom->Set_Position(vPos);
}

void CGrid::Set_Scale(const Vector3& vScale)
{
	m_pTransformCom->Set_Scale(vScale);
}

MATERIAL_INFO CGrid::Get_MaterialInfo()
{
	return m_pMaterialCom->Get_MaterialInfo();
}
