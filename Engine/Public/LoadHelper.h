#pragma once
#define NOMINMAX
#include "Base.h"


NAMESPACE_(Engine)
class CAssetMesh;
//struct aiNode;
//struct aiScene;
class ENGINE_DLL CLoadHelper : public CBase
{
	DECLARE_SINGLETON(CLoadHelper)

public:
	CLoadHelper() = default;
	~CLoadHelper() override = default;

public:
	HRESULT Initialize();

	void StartSign();
	HRESULT Load_Texture(const TEXTURE_LOAD_DESC& refTexture_load_desc, const wstring& strAssetName);
	void EndSign_Texture(); // Srv 생성 후의 오프셋부터 CBV 생성하도록

	//HRESULT Load_3DModel(const std::string& strPath, const std::string& strAssetName);
	/*void Recur_ProcessNode(aiNode* pNode, const aiScene* pScene, Matrix& refTr);
	CAssetMesh* Recur_ProcessMesh(aiMesh* pAiMesh, const aiScene* pScene);*/
	void EndSign_3DModel();

	HRESULT Free() override;

private:
	ID3D12Device* m_pDevice = nullptr;
	class CAssetManager* m_pAssetManager = nullptr;
	class ResourceUploadBatch* m_pResourceUpload = nullptr;
	std::future<void> m_future;
private: // Textures
	TEXTURE_INIT_DESC m_texture_init_desc{};
	UINT* m_pNextCbvSrvUavHeapOffset = nullptr;
	UINT m_iCbvSrvUavDescriptorSize = 0;
private: // Mesh
	std::list<CAssetMesh*> m_meshContainingList;
};

_NAMESPACE