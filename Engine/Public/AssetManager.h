#pragma once
#include "Base.h"

NAMESPACE_(Engine)
using namespace std;
class CAsset;
class CTexture;
class CMeshData;

class ENGINE_DLL CAssetManager : public CBase
{
	DECLARE_SINGLETON(CAssetManager)
	CAssetManager() = default;
	~CAssetManager() override = default;

public:
	HRESULT Free() override;
public:
	HRESULT Add_Texture(const wstring& strAssetName, CTexture* pTextureInstance);
	CTexture* FindandGet_Texture(const wstring& strAssetName);
public:
	HRESULT Add_MeshDataPrototype(const wstring& strPrototypeTag, CMeshData* pMeshData);
	CMeshData* FindandGet_MeshData(const wstring& strPrototypeTag);
	CMeshData* Clone_MeshData(const wstring& strPrototypeTag, void* pArg);
	HRESULT Add_MeshData_ClusteredPrototype(const wstring& strPrototypeTag, list<CMeshData*> meshDataList);
	list<CMeshData*>& FindandGet_MeshData_Clustered(const wstring& strPrototypeTag);
	list<CMeshData*> Clone_MeshData_Clustered(const wstring& strPrototypeTag);
private:
	map<wstring, CTexture*> m_mapTextures;
	map<wstring, CMeshData*> m_mapMeshData;
	map<wstring, list<CMeshData*>> m_mapMeshData_Clustered; // 여러 MeshData 모여있는
};


_NAMESPACE
