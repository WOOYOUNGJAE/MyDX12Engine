#include "LoadHelper.h"
#include "Graphic_Device.h"
#include "ResourceUploadBatch.h"
#include "AssetManager.h"
#include "AssetMesh.h"
#include "ComponentManager.h"
#include "Renderer.h"
#include "Texture.h"
#include "D3DResourceManager.h"


#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


#include "assimp\Importer.hpp"

#include "assimp\cimport.h"

#include "assimp\postprocess.h"

#include "assimp\scene.h"



#pragma comment(lib, "assimp.lib")

//#include <assimp\Importer.hpp>
//#include <assimp\postprocess.h>
//#include <assimp\scene.h>
//#include <filesystem>
//#include <memory>


IMPLEMENT_SINGLETON(CLoadHelper)

HRESULT CLoadHelper::Initialize()
{
	m_pDevice = CGraphic_Device::Get_Instance()->Get_Device();
	m_pAssetManager = CAssetManager::Get_Instance();

	m_pNextCbvSrvUavHeapOffset = CGraphic_Device::Get_Instance()->Get_NextCbvSrvUavHeapOffsetPtr();

	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pAssetManager);
	if (m_pDevice == nullptr)
	{
		MSG_BOX("LoadHelper: Device nullptr");
		return E_FAIL;
	}

	m_pResourceUpload = new ResourceUploadBatch(m_pDevice);

	m_texture_init_desc.pDevice = m_pDevice;
	m_texture_init_desc.pResourceUpload = m_pResourceUpload;

	m_iCbvSrvUavDescriptorSize = CGraphic_Device::Get_Instance()->Get_CbvSrvUavDescriptorSize();

	return S_OK;
}

void CLoadHelper::StartSign()
{
	CD3DResourceManager::Get_Instance()->Set_SrvOffsetStart((*m_pNextCbvSrvUavHeapOffset));
	CGraphic_Device::Get_Instance()->Reset_CmdList();
}

HRESULT CLoadHelper::Load_Texture(const TEXTURE_LOAD_DESC& refTexture_load_desc, const wstring& strAssetName)
{
	m_pResourceUpload->Begin();

	m_texture_init_desc.bIsCubeMap = refTexture_load_desc.bIsCubeMap;
	m_texture_init_desc.strPath = refTexture_load_desc.strPath;
	m_texture_init_desc.iCbvSrvUavHeapOffset = *m_pNextCbvSrvUavHeapOffset;

	m_pAssetManager->Add_Texture(strAssetName, CTexture::Create(&m_texture_init_desc));
	auto finish = m_pResourceUpload->End(CGraphic_Device::Get_Instance()->Get_CommandQueue());
	finish.wait();

	(*m_pNextCbvSrvUavHeapOffset) += m_iCbvSrvUavDescriptorSize;

	return S_OK;
}

void CLoadHelper::EndSign_Texture()
{
	CGraphic_Device::Get_Instance()->Close_CmdList();
	dynamic_cast<CRenderer*>(CComponentManager::Get_Instance()->FindandGet_Prototype(L"Renderer"))->Build_FrameResource();
	CD3DResourceManager::Get_Instance()->Set_SrvOffsetEnd((*m_pNextCbvSrvUavHeapOffset));
}
//
//HRESULT CLoadHelper::Load_3DModel(const string& strPath, const string& strAssetName)
//{
//	HRESULT hr = S_OK;
//
//	/*Assimp::Importer importer;
//
//	const aiScene* pScene = importer.ReadFile(
//	strPath.c_str(),
//		aiProcess_Triangulate | aiProcess_ConvertToLeftHanded);
//
//	Matrix mTr;
//	Recur_ProcessNode(pScene->mRootNode, pScene, mTr);*/
//
//	return hr;
//}
//
//void CLoadHelper::Recur_ProcessNode(aiNode* pNode, const aiScene* pScene, Matrix& refTr)
//{
//
//	/*Matrix m;
//	ai_real* temp = &pNode->mTransformation.a1;
//	float* mTemp = &m._11;
//	for (int t = 0; t < 16; t++) {
//		mTemp[t] = float(temp[t]);
//	}
//	m = m.Transpose() * refTr;
//
//	for (UINT i = 0; i < pNode->mNumMeshes; i++) 
//	{
//
//
//		aiMesh* pAiMesh = pScene->mMeshes[pNode->mMeshes[i]];
//		CAssetMesh* pGeneratedMesh = this->Recur_ProcessMesh(pAiMesh, pScene);
//
//
//
//		for (VertexPositionNormalTexture*& v : pGeneratedMesh->Get_vecVertices())
//		{
//			v->position = Vector3::Transform(v->position, m);
//		}
//
//		m_meshContainingList.emplace_back(pGeneratedMesh);
//	}
//
//	for (UINT i = 0; i < pNode->mNumChildren; i++)
//	{
//		this->Recur_ProcessNode(pNode->mChildren[i], pScene, m);
//	}*/
//}
//
//CAssetMesh* CLoadHelper::Recur_ProcessMesh(aiMesh* pAiMesh, const aiScene* pScene)
//{
//	return nullptr;
//}

void CLoadHelper::EndSign_3DModel()
{
}

HRESULT CLoadHelper::Free()
{
	Safe_Delete(m_pResourceUpload);

	Safe_Release(m_pAssetManager);
	Safe_Release(m_pDevice);

	return S_OK;
}

