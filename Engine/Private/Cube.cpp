#include "Cube.h"
#include "TextureCompo.h"
#include "MeshObject.h"
#include "Transform.h"
#include "Renderer.h"
#include "Shader.h"
#include "Material.h"
#include "MeshData.h"
#include "SDSManager.h"
#include "FrameResource.h"
CCube* CCube::Create()
{
	CCube* pInstance = new CCube;
	pInstance->m_bIsPrototype = true;
	pInstance->m_eGeometryType = GEOMETRY_TYPE::CUBE;
	if (pInstance)
	{
		pInstance->Initialize_Prototype();
	}

	return pInstance;
}

CGameObject* CCube::Clone(UINT* pInOutRenderNumbering, void* pArg)
{
	CCube* pInstance = new CCube(*this);

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

	MESHOBJ_INIT_DESC meshObjDesc { true, (UINT)m_eGeometryType, std::wstring() };
	hr = Add_Component(L"MeshObject", reinterpret_cast<CComponent**>(&m_pMeshObjectCom), &meshObjDesc);
	if (FAILED(hr)) return hr;
	hr = Add_Component(L"Renderer", reinterpret_cast<CComponent**>(&m_pRendererCom));
	if (FAILED(hr)) return hr;
	hr = Add_Component(L"Shader_Simple3", reinterpret_cast<CComponent**>(&m_pShaderCom));
	if (FAILED(hr)) return hr;
	//hr = Add_Component(L"Texture", reinterpret_cast<CComponent**>(&m_pTextureCom), &wstring(L"Texture_NightSkybox"));
	hr = Add_Component(L"Texture", reinterpret_cast<CComponent**>(&m_pTextureCom), &wstring(L"Texture_ice"));
	if (FAILED(hr)) return hr;

#if DXR_ON
	struct MATERIAL_INFO_2ELEMENTS
	{
		MATERIAL_INFO rastMatInfo;
		DXR::MATERIAL_INFO DXRMatInfo;
	}matInfo{};
	matInfo.rastMatInfo = { Vector3::Zero * 0.5f, 0.5f, Vector3::One * 0.5f, 0.f, Vector3::One * 0.5f };
	static int i = 0;
	if (i++ == 0)
	{
		matInfo.DXRMatInfo = { Vector4::Zero};
		matInfo.DXRMatInfo = { Vector4(1,0,0,1) };
	}
	else
	{
		matInfo.DXRMatInfo = { Vector4::One * 0.9f};
	}
#else
	MATERIAL_INFO matInfo{ Vector3::Zero * 0.5f, 0.5f, Vector3::One * 0.5f, 0.f, Vector3::One * 0.5f};
#endif
	hr = Add_Component(L"Material", reinterpret_cast<CComponent**>(&m_pMaterialCom), &matInfo);


	m_iTextureSrvOffset = m_pTextureCom->m_iCbvSrvUavHeapOffset;

	GAMEOBJECT_INIT_DESC* pCastedArg = static_cast<GAMEOBJECT_INIT_DESC*>(pArg);

	m_pTransformCom->Set_Position(pCastedArg->vStartPos);
	m_pTransformCom->Set_Look(pCastedArg->vStartLook);
	m_pTransformCom->Set_Scale(pCastedArg->vStartScale);
#if DXR_ON
	m_uqpBlAS = m_pMeshObjectCom->Move_BuiltBLAS();
	m_pUav_BLAS = m_uqpBlAS.get()->uav_BLAS;

	CSDSManager::Get_Instance()->Register_SceneNode(CGameObject::Make_NodeBLAS(), this, SDS_AS);
#endif

	return hr;
}

void CCube::Tick(_float fDeltaTime)
{
	m_pTransformCom->Rotate(m_pTransformCom->Up(), 0.5f * fDeltaTime);
	//m_pTransformCom->Move_Forward(0.5f * fDeltaTime);
}

void CCube::Late_Tick(_float fDeltaTime)
{
	int a = 0;
}

void CCube::Render_Tick()
{
	int a = 0;
	m_pRendererCom->AddTo_RenderGroup(RENDER_AFTER, CCW, NOBLEND, SHADERTYPE_SIMPLE3, ROOTSIG_DEFAULT, this);
}

void CCube::Render(ID3D12GraphicsCommandList* pCmdList, FrameResource* pFrameResource, UINT iRenderingElementIndex)
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
	m_pTransformCom->Set_Position(vPos);
}

void CCube::Set_Scale(const Vector3& vScale)
{
	m_pTransformCom->Set_Scale(vScale);
}

MATERIAL_INFO CCube::Get_MaterialInfo()
{
	return m_pMaterialCom->Get_MaterialInfo();
}
