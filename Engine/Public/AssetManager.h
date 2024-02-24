#pragma once
#include "Base.h"

NAMESPACE_(Engine)
using namespace std;
class CAsset;
class CTexture;
class CMeshData;

class CAssetManager : public CBase
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
private:
	map<wstring, CTexture*> m_mapTextures;
	map<wstring, CMeshData*> m_mapMeshData;
};


_NAMESPACE
