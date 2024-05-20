#include "pch.h"
#include "Skybox.h"

#include <Camera.h>
#include "FrameResource.h"
#include "TextureCompo.h"
#include "MeshObject.h"
#include "Transform.h"
#include "Renderer.h"
#include "Shader.h"
#include "Material.h"
#include "MeshData.h"
#include "CameraManager.h"

CSkybox* CSkybox::Create()
{
	CSkybox* pInstance = new CSkybox;
	pInstance->m_bIsPrototype = true;
	if (pInstance)
	{
		pInstance->Initialize_Prototype();
	}

	return pInstance;
}

CGameObject* CSkybox::Clone(UINT* pInOutRenderNumbering, void* pArg)
{
	CSkybox* pInstance = new CSkybox(*this);

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

HRESULT CSkybox::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CSkybox::Initialize(void* pArg)
{
	HRESULT hr = S_OK;
	GAMEOBJECT_INIT_DESC* pCastedInitDesc = reinterpret_cast<GAMEOBJECT_INIT_DESC*>(pArg);
	// ±âº» ÄÄÆ÷³ÍÆ® ºÎÂø
	hr = Add_Component(L"Transform", reinterpret_cast<CComponent**>(&m_pTransformCom));
	if (FAILED(hr)) return hr;
	/*hr = Add_Component(L"TriangleMesh", reinterpret_cast<CComponent**>(&m_pTriangleMeshCom));
	if (FAILED(hr)) return hr;*/

	MESHOBJ_INIT_DESC meshObjDesc{ true, L"CubeMesh" };
	hr = Add_Component(L"MeshObject", reinterpret_cast<CComponent**>(&m_pMeshObjectCom), &meshObjDesc);
	if (FAILED(hr)) return hr;
	hr = Add_Component(L"Renderer", reinterpret_cast<CComponent**>(&m_pRendererCom));
	if (FAILED(hr)) return hr;
	hr = Add_Component(L"Shader_skybox", reinterpret_cast<CComponent**>(&m_pShaderCom));
	if (FAILED(hr)) return hr;
	hr = Add_Component(L"Texture", reinterpret_cast<CComponent**>(&m_pTextureCom), &wstring(L"Texture_NightSkybox"));
	//hr = Add_Component(L"Texture", reinterpret_cast<CComponent**>(&m_pTextureCom), &wstring(L"Texture_ice"));
	if (FAILED(hr)) return hr;

	MATERIAL_INFO matInfo{ Vector3::Zero * 0.5f, 0.5f, Vector3::One * 0.5f, 0.f, Vector3::One * 0.5f };
	hr = Add_Component(L"Material", reinterpret_cast<CComponent**>(&m_pMaterialCom), &matInfo);


	m_iTextureSrvOffset = m_pTextureCom->m_iCbvSrvUavHeapOffset;

	m_pTransformCom->Set_Position(pCastedInitDesc->vStartPos);
	m_pTransformCom->Set_Scale(pCastedInitDesc->vStartScale);

	return hr;
}

void CSkybox::Tick(_float fDeltaTime)
{

}

void CSkybox::Late_Tick(_float fDeltaTime)
{
	m_pTransformCom->Set_Position(CCameraManager::Get_Instance()->Get_MainCam()->Get_Pos());
}

void CSkybox::Render_Tick()
{
	m_pRendererCom->AddTo_RenderGroup( RENDER_FIRST, RENDER_CULLMODE::NONE, NOBLEND, SHADERTYPE_SKYBOX, ROOTSIG_DEFAULT, this);
}

void CSkybox::Render(ID3D12GraphicsCommandList* pCmdList, FrameResource* pFrameResource, UINT iRenderingElementIndex)
{
	// Update CB
	OBJECT_CB objConstants;
	objConstants.mWorldMat = Get_WorldMatrix().Transpose();
	objConstants.mInvTranspose = Get_WorldMatrix().Invert(); // Transpose µÎ¹ø
	objConstants.material = Get_MaterialInfo();
	pFrameResource->pObjectCB->CopyData(iRenderingElementIndex, objConstants);

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



HRESULT CSkybox::Free()
{
	Safe_Release(m_pMaterialCom);
	Safe_Release(m_pTextureCom);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pRendererCom);
	Safe_Release(m_pMeshObjectCom);
	Safe_Release(m_pTransformCom);

	return CGameObject::Free();
}

Matrix CSkybox::Get_WorldMatrix()
{
	return m_pTransformCom->WorldMatrix();
}

Vector3 CSkybox::Get_Pos()
{
	return m_pTransformCom->Position();
}

Vector3 CSkybox::Get_ScaleXYZ()
{
	return CGameObject::Get_ScaleXYZ();
}

void CSkybox::Set_Position(const Vector3& vPos)
{
	m_pTransformCom->Set_Position(vPos);
}

void CSkybox::Set_Scale(const Vector3& vScale)
{
	m_pTransformCom->Set_Scale(vScale);
}

MATERIAL_INFO CSkybox::Get_MaterialInfo()
{
	return m_pMaterialCom->Get_MaterialInfo();
}
