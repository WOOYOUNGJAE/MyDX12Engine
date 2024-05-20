#include "pch.h"
#include "ZeldaDemo.h"
#include "MeshObject.h"
#include "Transform.h"
#include "Renderer.h"
#include "Shader.h"
#include "Material.h"
#include "MeshData.h"
#include "FrameResource.h"
CZeldaDemo::CZeldaDemo(CZeldaDemo& rhs) : CGameObject(rhs)
{
}

CZeldaDemo* CZeldaDemo::Create()
{
	CZeldaDemo* pInstance = new CZeldaDemo;
	pInstance->m_bIsPrototype = true;
	if (pInstance)
	{
		pInstance->Initialize_Prototype();
	}

	return pInstance;
}

CGameObject* CZeldaDemo::Clone(UINT* pInOutRenderNumbering, void* pArg)
{
	CZeldaDemo* pInstance = new CZeldaDemo(*this);

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

HRESULT CZeldaDemo::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CZeldaDemo::Initialize(void* pArg)
{
	HRESULT hr = S_OK;
	// ±âº» ÄÄÆ÷³ÍÆ® ºÎÂø
	hr = Add_Component(L"Transform", reinterpret_cast<CComponent**>(&m_pTransformCom));
	if (FAILED(hr)) return hr;
	MESHOBJ_INIT_DESC meshInitDesc{ false, L"zeldaPosed001" };
	hr = Add_Component(L"MeshObject", reinterpret_cast<CComponent**>(&m_pMeshObjectCom), &meshInitDesc);
	if (FAILED(hr)) return hr;
	hr = Add_Component(L"Renderer", reinterpret_cast<CComponent**>(&m_pRendererCom));
	if (FAILED(hr)) return hr;
	hr = Add_Component(L"Shader_Simple2", reinterpret_cast<CComponent**>(&m_pShaderCom));
	if (FAILED(hr)) return hr;


	MATERIAL_INFO matInfo{ Vector3::Zero * 0.5f, 0.5f, Vector3::One * 0.5f, 0.f, Vector3::One * 0.5f };
	hr = Add_Component(L"Material", reinterpret_cast<CComponent**>(&m_pMaterialCom), &matInfo);


	m_iTextureSrvOffset = m_pMeshObjectCom->Get_CbvSrvUavOffset(0);

	m_pTransformCom->Set_Position(static_cast<GAMEOBJECT_INIT_DESC*>(pArg)->vStartPos);

	m_pTransformCom->Set_Scale(Vector3(0.01, 0.01, 0.01));

	return hr;
}

void CZeldaDemo::Tick(_float fDeltaTime)
{
}

void CZeldaDemo::Late_Tick(_float fDeltaTime)
{
}

void CZeldaDemo::Render_Tick()
{
	m_pRendererCom->AddTo_RenderGroup(RENDER_AFTER, RENDER_CULLMODE::CCW, NOBLEND, SHADERTYPE_SIMPLE3, ROOTSIG_DEFAULT, this);
}

void CZeldaDemo::Render(ID3D12GraphicsCommandList* pCmdList, FrameResource* pFrameResource, UINT iRenderingElementIndex)
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
		pCmdList->SetGraphicsRootDescriptorTable(0, m_pRendererCom->Get_HandleOffsettedGPU((INT)pMesh->Get_CbvSrvUavOffset()));
		pCmdList->SetGraphicsRootConstantBufferView(1, objCBAddress);

		//pCmdList->DrawInstanced(24, 1, 0, 0);
		pCmdList->DrawIndexedInstanced(
			pMesh->Num_Indices(),
			1,
			0,
			0,
			0);
	}
}

HRESULT CZeldaDemo::Free()
{
	Safe_Release(m_pMaterialCom);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pRendererCom);
	Safe_Release(m_pMeshObjectCom);
	Safe_Release(m_pTransformCom);

	return CGameObject::Free();
}

MATERIAL_INFO CZeldaDemo::Get_MaterialInfo()
{
	return m_pMaterialCom->Get_MaterialInfo();
}

Matrix CZeldaDemo::Get_WorldMatrix()
{
	return m_pTransformCom->WorldMatrix();
}

Vector3 CZeldaDemo::Get_Pos()
{
	return m_pTransformCom->Position();
}

Vector3 CZeldaDemo::Get_ScaleXYZ()
{
	return CGameObject::Get_ScaleXYZ();

}

void CZeldaDemo::Set_Position(const Vector3& vPos)
{
	m_pTransformCom->Set_Position(vPos);
}

void CZeldaDemo::Set_Scale(const Vector3& vScale)
{
	m_pTransformCom->Set_Scale(vScale);
}
