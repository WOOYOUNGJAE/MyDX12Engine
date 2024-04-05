#include "LoadHelper.h"
#include "DeviceResource.h"
#include "ResourceUploadBatch.h"
#include "AssetManager.h"
#include "AssetMesh.h"
#include "ComponentManager.h"
#include "Renderer.h"
#include "Texture.h"
//#include "D3DResourceManager.h"
#include "FrameResourceManager.h"
#include "MeshData.h"

IMPLEMENT_SINGLETON(CLoadHelper)

HRESULT CLoadHelper::Initialize()
{
	m_pDevice = CDeviceResource::Get_Instance()->Get_Device();
	m_pAssetManager = CAssetManager::Get_Instance();

	m_pNextCbvSrvUavHeapOffset = CDeviceResource::Get_Instance()->Get_NextCbvSrvUavHeapOffsetPtr();

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

	m_iCbvSrvUavDescriptorSize = CDeviceResource::Get_Instance()->Get_CbvSrvUavDescriptorSize();

	return S_OK;
}

void CLoadHelper::StartSign()
{

	//CD3DResourceManager::Get_Instance()->Set_SrvOffsetStart((*m_pNextCbvSrvUavHeapOffset));
	CDeviceResource::Get_Instance()->Reset_CmdList();
}

HRESULT CLoadHelper::Load_Texture(const TEXTURE_LOAD_DESC& refTexture_load_desc, const wstring& strAssetName)
{
	m_pResourceUpload->Begin();

	m_texture_init_desc.bIsCubeMap = refTexture_load_desc.bIsCubeMap;
	m_texture_init_desc.strPath = refTexture_load_desc.strPath;
	m_texture_init_desc.iCbvSrvUavHeapOffset = *m_pNextCbvSrvUavHeapOffset;

	m_pAssetManager->Add_Texture(strAssetName, CTexture::Create(&m_texture_init_desc));
	auto finish = m_pResourceUpload->End(CDeviceResource::Get_Instance()->Get_CommandQueue());
	finish.wait();

	(*m_pNextCbvSrvUavHeapOffset) += m_iCbvSrvUavDescriptorSize;

	return S_OK;
}

void CLoadHelper::EndSign()
{
	CDeviceResource::Get_Instance()->Close_CmdList();
	CDeviceResource::Get_Instance()->Execute_CmdList();
	dynamic_cast<CRenderer*>(CComponentManager::Get_Instance()->FindandGet_Prototype(L"Renderer"))->Build_FrameResource();
	CFrameResourceManager::Get_Instance()->Build_FrameResource();
	//CD3DResourceManager::Get_Instance()->Set_SrvOffsetEnd((*m_pNextCbvSrvUavHeapOffset));
}

// ----------------------------------------------------------------------------------

HRESULT CLoadHelper::Load_3DModel(const string& strPath, const string& strAssetName, list<CMeshData*>* pOutMeshList)
{
	HRESULT hr = S_OK;

	m_strBasePath = strPath;

	Assimp::Importer importer;

	const aiScene* pScene = importer.ReadFile(
	(strPath + strAssetName).c_str(),
		aiProcess_Triangulate | aiProcess_ConvertToLeftHanded);

	Matrix mTr;
	Recur_ProcessNode(pScene->mRootNode, pScene, mTr);

	*pOutMeshList = std::move(m_meshContainingList);

	for (CMeshData*& iterMesh : *pOutMeshList)
	{
		// 다른 메쉬가 같은 텍스처 공유하는 경우 있음
		CAssetMesh* pDownCastedMesh = dynamic_cast<CAssetMesh*>(iterMesh);
		hr = pDownCastedMesh->ReInit_Prototype();
		if (FAILED(hr)) { return E_FAIL; }

		// Textures
		wstring wstrAssetName = pDownCastedMesh->Get_Path();
		// 3DModels포함해서 이전 경로 삭제
		size_t extIndex = wstrAssetName.find(L"3DModels");
		if (extIndex != wstring::npos)
		{
			wstrAssetName.erase(0, extIndex + wcslen(L"3DModels") + 1);
		}
		// 확장자 제거
		extIndex = wstrAssetName.rfind(L'.');
		if (extIndex != wstring::npos)
		{
			wstrAssetName.erase(extIndex, wstrAssetName.size() - extIndex + 1);
		}

		CTexture* pInstance = m_pAssetManager->FindandGet_Texture(wstrAssetName);
		if (pInstance) // 이미 있다면 텍스처 로드는 건너 뜀 (다른 메쉬가 같은 텍스처를 가리키는 경우)
		{
			iterMesh->Set_CbvSrvUavOffset(pInstance->m_iCbvSrvUavHeapOffset);
			continue;
		}
		pInstance = nullptr;

		// Texture Load
		m_pResourceUpload->Begin();

		m_texture_init_desc.bIsCubeMap = false;
		m_texture_init_desc.strPath = pDownCastedMesh->Get_Path();
		m_texture_init_desc.iCbvSrvUavHeapOffset = *m_pNextCbvSrvUavHeapOffset;

		hr = m_pAssetManager->Add_Texture(wstrAssetName, CTexture::Create(&m_texture_init_desc));
		if (FAILED(hr))
		{
			MSG_BOX("Craeting Texture Failed");
			return hr;
		}
		iterMesh->Set_CbvSrvUavOffset(*m_pNextCbvSrvUavHeapOffset);

		auto finish = m_pResourceUpload->End(CDeviceResource::Get_Instance()->Get_CommandQueue());
		finish.wait();

		(*m_pNextCbvSrvUavHeapOffset) += m_iCbvSrvUavDescriptorSize;
	}
	CDeviceResource::Get_Instance()->Close_CmdList();
	CDeviceResource::Get_Instance()->Execute_CmdList();
	CDeviceResource::Get_Instance()->Reset_CmdList();


	return hr;
}

void CLoadHelper::Recur_ProcessNode(aiNode* pNode, const aiScene* pScene, Matrix& refTr)
{
	Matrix m;
	ai_real* temp = &pNode->mTransformation.a1;
	float* mTemp = &m._11;
	for (int t = 0; t < 16; t++) {
		mTemp[t] = float(temp[t]);
	}
	m = m.Transpose() * refTr;

	for (UINT i = 0; i < pNode->mNumMeshes; i++) 
	{
		aiMesh* pAiMesh = pScene->mMeshes[pNode->mMeshes[i]];
		CMeshData* pGeneratedMesh = this->Recur_ProcessMesh(pAiMesh, pScene);

		for (VertexPositionNormalTexture& v : dynamic_cast<CAssetMesh*>(pGeneratedMesh)->Get_vecVertices())
		{
			v.position = Vector3::Transform(v.position, m);
		}

		m_meshContainingList.emplace_back(pGeneratedMesh);
	}

	for (UINT i = 0; i < pNode->mNumChildren; i++)
	{
		this->Recur_ProcessNode(pNode->mChildren[i], pScene, m);
	}
}

CMeshData* CLoadHelper::Recur_ProcessMesh(aiMesh* pAiMesh, const aiScene* pScene)
{
	std::vector<VertexPositionNormalTexture> vertices;
	std::vector<UINT32> indices;

	// Walk through each of the mesh's vertices
	for (UINT i = 0; i < pAiMesh->mNumVertices; i++) {
		VertexPositionNormalTexture vertex;

		vertex.position.x = pAiMesh->mVertices[i].x;
		vertex.position.y = pAiMesh->mVertices[i].y;
		vertex.position.z = pAiMesh->mVertices[i].z;

		vertex.normal.x = pAiMesh->mNormals[i].x;
		vertex.normal.y = pAiMesh->mNormals[i].y;
		vertex.normal.z = pAiMesh->mNormals[i].z;

		XMVECTOR xmvNormalized = XMVector3Normalize(XMLoadFloat3(&vertex.normal));
		XMStoreFloat3(&vertex.normal, xmvNormalized);


		if (pAiMesh->mTextureCoords[0]) {
			vertex.textureCoordinate.x = (float)pAiMesh->mTextureCoords[0][i].x;
			vertex.textureCoordinate.y = (float)pAiMesh->mTextureCoords[0][i].y;
		}

		vertices.push_back(vertex);
	}

	for (UINT i = 0; i < pAiMesh->mNumFaces; i++) {
		aiFace face = pAiMesh->mFaces[i];
		for (UINT j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}

	CMeshData* pGeneratedMesh = CAssetMesh::Create();
	CAssetMesh* pDownCastedMesh = dynamic_cast<CAssetMesh*>(pGeneratedMesh);

	pDownCastedMesh->Get_vecVertices() = std::move(vertices);
	pDownCastedMesh->Get_vecIndices() = std::move(indices);

	// http://assimp.sourceforge.net/lib_html/materials.html
	if (pAiMesh->mMaterialIndex >= 0) {
		aiMaterial* material = pScene->mMaterials[pAiMesh->mMaterialIndex];

		if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
			aiString filepath;
			material->GetTexture(aiTextureType_DIFFUSE, 0, &filepath);

			std::string fullPath =
				this->m_strBasePath +
				std::string(std::filesystem::path(filepath.C_Str())
					.filename()
					.string());

			// string to wstring
			pDownCastedMesh->Get_Path().assign(fullPath.begin(), fullPath.end());
		}
	}

	return pGeneratedMesh;
}

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

