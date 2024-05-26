#include "Triangle.h"

#include "Camera.h"
#include "CameraManager.h"
#include "TriangleMesh.h"
#include "TriangleMesh_PT.h"
#include "Transform.h"
#include "Renderer.h"
#include "Shader.h"
#include "TextureCompo.h"
#include "MeshObject.h"
#include "SDSManager.h"
#include "FrameResource.h"
#include "material.h"

CTriangle* CTriangle::Create()
{
	CTriangle* pInstance = new CTriangle;
	pInstance->m_bIsPrototype = true;
	pInstance->m_eGeometryType = GEOMETRY_TYPE::TRIANGLE;
	if (pInstance)
	{
		pInstance->Initialize_Prototype();
	}

	return pInstance;
}

CGameObject* CTriangle::Clone(UINT* pInOutRenderNumbering, void* pArg)
{
	CTriangle* pInstance = new CTriangle(*this);

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
	MESHOBJ_INIT_DESC meshObjDesc{ true, (UINT)m_eGeometryType, std::wstring()};
	hr = Add_Component(L"MeshObject", reinterpret_cast<CComponent**>(&m_pMeshObjectCom), &meshObjDesc);
	if (FAILED(hr)) return hr;
	hr = Add_Component(L"Renderer", reinterpret_cast<CComponent**>(&m_pRendererCom));
	if (FAILED(hr)) return hr;
	hr = Add_Component(L"Shader_Simple2", reinterpret_cast<CComponent**>(&m_pShaderCom));
	if (FAILED(hr)) return hr;
	hr = Add_Component(L"Texture", reinterpret_cast<CComponent**>(&m_pTextureCom), &wstring(L"Texture_Checkboard"));
	//hr = Add_Component(L"Texture", reinterpret_cast<CComponent**>(&m_pTextureCom), &wstring(L"Texture_ice"));
	if (FAILED(hr)) return hr;

#if DXR_ON
	struct MATERIAL_INFO_2ELEMENTS
	{
		MATERIAL_INFO rastMatInfo;
		DXR::MATERIAL_INFO DXRMatInfo = DXR::MATERIAL_INFO();
	}matInfo{};
	matInfo.rastMatInfo = { Vector3::Zero * 0.5f, 0.5f, Vector3::One * 0.5f, 0.f, Vector3::One * 0.5f };
	
#else
	MATERIAL_INFO matInfo{ Vector3::Zero * 0.5f, 0.5f, Vector3::One * 0.5f, 0.f, Vector3::One * 0.5f };
#endif
	hr = Add_Component(L"Material", reinterpret_cast<CComponent**>(&m_pMaterialCom), &matInfo);

	m_iTextureSrvOffset = m_pTextureCom->m_iCbvSrvUavHeapOffset;

	GAMEOBJECT_INIT_DESC* pCastedArg = static_cast<GAMEOBJECT_INIT_DESC*>(pArg);

	m_pTransformCom->Set_Position(pCastedArg->vStartPos);
	m_pTransformCom->Set_Look(pCastedArg->vStartLook);
	m_pTransformCom->Set_Scale(pCastedArg->vStartScale);

#if DXR_ON
	CSDSManager::Get_Instance()->Register_SceneNode(CGameObject::Make_NodeBLAS(), this, SDS_AS);
#endif
	return hr;
}

void CTriangle::Tick(_float fDeltaTime)
{
	//auto cam = CCameraManager::Get_Instance()->Get_MainCam();
	//float fDistance = 1.f;
	//Vector3 camLook = -cam->Get_WorldMatrix().Forward();
	//auto a = cam->Get_WorldMatrix();
	////Set_Position(cam->Get_Pos() + camLook);
	//Vector3 newPos = cam->Get_Pos() + camLook * fDistance;
	//Set_Position(newPos);
	//m_pTransformCom->Set_Look(camLook);

	//
	//PRINT_VECTOR3(camLook);
	//PRINT_VECTOR3(-Get_WorldMatrix().Forward());
	//cout << endl;

	/*m_pTransformCom->m_vPosition += Vector3(0.001f, 0.f, 0.f);
	m_pTransformCom->Refresh_WorldMatrix(CTransform::POSITION);*/
}

void CTriangle::Late_Tick(_float fDeltaTime)
{
	int a = 0;
}

void CTriangle::Render_Tick()
{
	// Bind Resource
	//m_pRendererCom->AddTo_RenderGroup(RENDER_AFTER, NOBLEND, SHADERTYPE_SIMPLE, ROOTSIG_DEFAULT, this);
	m_pRendererCom->AddTo_RenderGroup(RENDER_AFTER, CCW, NOBLEND, SHADERTYPE_SIMPLE3, ROOTSIG_DEFAULT, this);
}

void CTriangle::Render(ID3D12GraphicsCommandList* pCmdList, FrameResource* pFrameResource, UINT iRenderingElementIndex)
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

HRESULT CTriangle::Free()
{
	Safe_Release(m_pMaterialCom);
	Safe_Release(m_pTextureCom);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pRendererCom);
	Safe_Release(m_pMeshObjectCom);
	Safe_Release(m_pTransformCom);

	return CGameObject::Free();
}

Matrix CTriangle::Get_WorldMatrix()
{
	return m_pTransformCom->WorldMatrix();
}

Vector3 CTriangle::Get_Pos()
{
	return m_pTransformCom->Position();
}

Vector3 CTriangle::Get_ScaleXYZ()
{
	return m_pTransformCom->Get_ScaleXYZ();
}

void CTriangle::Set_Position(const Vector3& vPos)
{
	m_pTransformCom->Set_Position(vPos);
}

void CTriangle::Set_Scale(const Vector3& vScale)
{
	m_pTransformCom->Set_Scale(vScale);
}
