#include "pch.h"
#include "TreeBillboard.h"

#include <Camera.h>


#include "TextureCompo.h"
#include "MeshObject.h"
#include "Transform.h"
#include "Renderer.h"
#include "Shader.h"
#include "Material.h"
#include "MeshData.h"
#include "CameraManager.h"


CTreeBillboard* CTreeBillboard::Create()
{
	CTreeBillboard* pInstance = new CTreeBillboard;
	pInstance->m_bIsPrototype = true;
	if (pInstance)
	{
		pInstance->Initialize_Prototype();
	}

	return pInstance;
}

CGameObject* CTreeBillboard::Clone(void* pArg)
{
	CTreeBillboard* pInstance = new CTreeBillboard(*this);

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

HRESULT CTreeBillboard::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CTreeBillboard::Initialize(void* pArg)
{
	HRESULT hr = S_OK;

	GAMEOBJECT_INIT_DESC* pCastedInitDesc = reinterpret_cast<GAMEOBJECT_INIT_DESC*>(pArg);

	// ±âº» ÄÄÆ÷³ÍÆ® ºÎÂø
	hr = Add_Component(L"Transform", reinterpret_cast<CComponent**>(&m_pTransformCom));
	if (FAILED(hr)) return hr;
	MESHOBJ_INIT_DESC meshObjDesc{ true, L"SingleVertexMesh" };
	hr = Add_Component(L"MeshObject", reinterpret_cast<CComponent**>(&m_pMeshObjectCom), &meshObjDesc);
	if (FAILED(hr)) return hr;
	hr = Add_Component(L"Renderer", reinterpret_cast<CComponent**>(&m_pRendererCom));
	if (FAILED(hr)) return hr;
	hr = Add_Component(L"Shader_treeBillboard", reinterpret_cast<CComponent**>(&m_pShaderCom));
	if (FAILED(hr)) return hr;
	hr = Add_Component(L"Texture", reinterpret_cast<CComponent**>(&m_pTextureCom), &wstring(L"Texture_Tree0"));
	if (FAILED(hr)) return hr;

	MATERIAL_INFO matInfo{ Vector3::Zero * 0.5f, 0.5f, Vector3::One * 0.5f, 0.f, Vector3::One * 0.5f };
	hr = Add_Component(L"Material", reinterpret_cast<CComponent**>(&m_pMaterialCom), &matInfo);

	m_iTextureSrvOffset = m_pTextureCom->m_iCbvSrvUavHeapOffset;

	m_pTransformCom->Set_Position(pCastedInitDesc->vStartPos);
	m_pTransformCom->Set_Scale(pCastedInitDesc->vStartScale);

	return hr;
}

void CTreeBillboard::Tick(_float fDeltaTime)
{
	Vector3 vCamPos = CCameraManager::Get_Instance()->Get_MainCam()->Get_Pos();
	Vector3 vToCamPos = vCamPos - Get_Pos();
	vToCamPos.y = 0;
	vToCamPos.Normalize();

	m_pTransformCom->Set_Look(vToCamPos);

	/*Matrix mat = CCameraManager::Get_Instance()->Get_MainCam()->Get_WorldMatrix();
	m_pTransformCom->Set_WorldMatrix(CTransform::MAT_RIGHT, mat.Right());
	m_pTransformCom->Set_WorldMatrix(CTransform::MAT_UP, mat.Up());
	m_pTransformCom->Set_WorldMatrix(CTransform::MAT_LOOK, -mat.Forward());*/

}

void CTreeBillboard::Late_Tick(_float fDeltaTime)
{
}

void CTreeBillboard::Render_Tick()
{
	m_pRendererCom->AddTo_RenderGroup(RENDER_CULLMODE::NONE, RENDER_AFTER, NOBLEND, SHADERTYPE_TREEBILLBOARD, ROOTSIG_DEFAULT, this);
}

void CTreeBillboard::Render(ID3D12GraphicsCommandList* pCmdList, FrameResource* pFrameResource, UINT iRenderingElementIndex)
{
	// Update CB
	OBJECT_CB objConstants;
	objConstants.mWorldMat = Get_WorldMatrix().Transpose();
	objConstants.mInvTranspose = Get_WorldMatrix().Invert(); // Transpose µÎ¹ø
	objConstants.material = Get_MaterialInfo();
	pFrameResource->pObjectCB->CopyData(iRenderingElementIndex, objConstants);

	pCmdList->IASetPrimitiveTopology(PrimitiveType()); // PointList

	// for each meshes
	CMeshData* pMesh = (*(m_pMeshObjectCom->Get_vecMeshData()))[0];

	UINT objCBByteSize = CDevice_Utils::ConstantBufferByteSize(sizeof(OBJECT_CB));
	D3D12_GPU_VIRTUAL_ADDRESS objCBAddress =
		pFrameResource->pObjectCB->Get_UploadBuffer()->GetGPUVirtualAddress() + iRenderingElementIndex * objCBByteSize;

	pCmdList->IASetVertexBuffers(0, 1, pMesh->Get_VertexBufferViewPtr());
	pCmdList->IASetIndexBuffer(pMesh->Get_IndexBufferViewPtr());

	// Set Descriptor Tables
	pCmdList->SetGraphicsRootDescriptorTable(0, m_pRendererCom->Get_HandleOffsettedGPU(Get_CbvSrvUavHeapOffset_Texture()));
	pCmdList->SetGraphicsRootConstantBufferView(1, objCBAddress);

	pCmdList->DrawIndexedInstanced(
		pMesh->Num_Indices(),
		1,
		0,
		0,
		0);

}



HRESULT CTreeBillboard::Free()
{
	Safe_Release(m_pMaterialCom);
	Safe_Release(m_pTextureCom);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pRendererCom);
	Safe_Release(m_pMeshObjectCom);
	Safe_Release(m_pTransformCom);

	return CGameObject::Free();
}

Matrix CTreeBillboard::Get_WorldMatrix()
{
	return m_pTransformCom->WorldMatrix();
}

Vector3 CTreeBillboard::Get_Pos()
{
	return m_pTransformCom->Position();
}

Vector3 CTreeBillboard::Get_ScaleXYZ()
{
	return CGameObject::Get_ScaleXYZ();
}

void CTreeBillboard::Set_Position(const Vector3& vPos)
{
	m_pTransformCom->Set_Position(vPos);
}

void CTreeBillboard::Set_Scale(const Vector3& vScale)
{
	m_pTransformCom->Set_Scale(vScale);
}

MATERIAL_INFO CTreeBillboard::Get_MaterialInfo()
{
	return m_pMaterialCom->Get_MaterialInfo();
}
