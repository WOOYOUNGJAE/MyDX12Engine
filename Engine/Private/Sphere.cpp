#include "Sphere.h"
#include "TextureCompo.h"
#include "MeshObject.h"
#include "Transform.h"
#include "Renderer.h"
#include "Shader.h"
#include "Material.h"
#include "MeshData.h"

CSphere::CSphere(CSphere& rhs) : CGameObject(rhs),
m_strSphereMeshTag(rhs.m_strSphereMeshTag)
{
}

CSphere* CSphere::Create(const wstring& strGridMeshTag)
{
	CSphere* pInstance = new CSphere;
	if (strGridMeshTag.empty())
	{
		MSG_BOX("CSphere : Enter Grid Tag");
		return nullptr;
	}
	else
	{
		pInstance->m_strSphereMeshTag = strGridMeshTag;
	}

	pInstance->m_bIsPrototype = true;
	if (pInstance)
	{
		pInstance->Initialize_Prototype();
	}

	return pInstance;
}

CGameObject* CSphere::Clone(void* pArg)
{
	CSphere* pInstance = new CSphere(*this);

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

HRESULT CSphere::Initialize_Prototype()
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT CSphere::Initialize(void* pArg)
{
	HRESULT hr = S_OK;
	// ±âº» ÄÄÆ÷³ÍÆ® ºÎÂø
	hr = Add_Component(L"Transform", reinterpret_cast<CComponent**>(&m_pTransformCom));
	if (FAILED(hr)) return hr;
	/*hr = Add_Component(L"TriangleMesh", reinterpret_cast<CComponent**>(&m_pTriangleMeshCom));
	if (FAILED(hr)) return hr;*/
	MESHOBJ_INIT_DESC meshObjDesc{ true, m_strSphereMeshTag };
	hr = Add_Component(L"MeshObject", reinterpret_cast<CComponent**>(&m_pMeshObjectCom), &meshObjDesc);
	if (FAILED(hr)) return hr;


	hr = Add_Component(L"Renderer", reinterpret_cast<CComponent**>(&m_pRendererCom));
	if (FAILED(hr)) return hr;
	hr = Add_Component(L"Shader_Simple2", reinterpret_cast<CComponent**>(&m_pShaderCom));
	if (FAILED(hr)) return hr;
	hr = Add_Component(L"Texture", reinterpret_cast<CComponent**>(&m_pTextureCom), &wstring(L"Texture_NightSkybox"));
	//hr = Add_Component(L"Texture", reinterpret_cast<CComponent**>(&m_pTextureCom), &wstring(L"Texture_ice"));
	if (FAILED(hr)) return hr;

	MATERIAL_INFO matInfo{ Vector3::Zero * 0.5f, 0.5f, Vector3::One * 0.5f, 0.f, Vector3::One * 0.5f };
	hr = Add_Component(L"Material", reinterpret_cast<CComponent**>(&m_pMaterialCom), &matInfo);


	m_iTextureSrvOffset = m_pTextureCom->m_iCbvSrvUavHeapOffset;

	m_pTransformCom->Set_Position(static_cast<GAMEOBJECT_INIT_DESC*>(pArg)->vStartPos);
	m_pTransformCom->Set_Scale(static_cast<GAMEOBJECT_INIT_DESC*>(pArg)->vStartScale);

	return hr;
}

void CSphere::Tick(_float fDeltaTime)
{
	int a = 0;
}

void CSphere::Late_Tick(_float fDeltaTime)
{
	int a = 0;
}

void CSphere::Render_Tick()
{
	int a = 0;
	m_pRendererCom->AddTo_RenderGroup(RENDER_CULLMODE::NONE, RENDER_AFTER, NOBLEND, SHADERTYPE_SIMPLE3, ROOTSIG_DEFAULT, this);
}

void CSphere::Render(ID3D12GraphicsCommandList* pCmdList, FrameResource* pFrameResource)
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

		UINT objCBByteSize = CDevice_Utils::ConstantBufferByteSize(sizeof(OBJECT_CB));
		D3D12_GPU_VIRTUAL_ADDRESS objCBAddress =
			pFrameResource->pObjectCB->Get_UploadBuffer()->GetGPUVirtualAddress() + (m_iClonedNum - 1) * objCBByteSize;

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



HRESULT CSphere::Free()
{
	Safe_Release(m_pMaterialCom);
	Safe_Release(m_pTextureCom);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pRendererCom);
	Safe_Release(m_pMeshObjectCom);
	Safe_Release(m_pTransformCom);

	return CGameObject::Free();
}

Matrix CSphere::Get_WorldMatrix()
{
	return m_pTransformCom->WorldMatrix();
}

Vector3 CSphere::Get_Pos()
{
	return m_pTransformCom->Position();
}

Vector3 CSphere::Get_ScaleXYZ()
{
	return CGameObject::Get_ScaleXYZ();
}

void CSphere::Set_Position(const Vector3& vPos)
{
	m_pTransformCom->Set_Position(vPos);
}

void CSphere::Set_Scale(const Vector3& vScale)
{
	m_pTransformCom->Set_Scale(vScale);
}

MATERIAL_INFO CSphere::Get_MaterialInfo()
{
	return m_pMaterialCom->Get_MaterialInfo();
}
