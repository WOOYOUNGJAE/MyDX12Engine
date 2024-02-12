#pragma once
#include "Base.h"

NAMESPACE_(Engine)
using namespace std;
class CAsset;
class CTexture;

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

private:
	map<wstring, CTexture*> m_mapTextures;
};


_NAMESPACE
