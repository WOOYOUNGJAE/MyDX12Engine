#pragma once
#include "Base.h"

NAMESPACE_(Engine)

class ENGINE_DLL CLoadHelper : public CBase
{
	DECLARE_SINGLETON(CLoadHelper)

public:
	CLoadHelper() = default;
	~CLoadHelper() override = default;

public:
	HRESULT Initialize();
	void LoadTextures_Begin();
	HRESULT Load_Texture(const TEXTURE_LOAD_DESC& refTexture_load_desc, const wstring& strPrototypeTag);
	HRESULT Free() override;

private:
	ID3D12Device* m_pDevice = nullptr;
	class CComponentManager* m_pComponentManager = nullptr;
	class ResourceUploadBatch* m_pResourceUpload = nullptr;
	std::future<void> m_future;
private: // Textures
	TEXTURE_INIT_DESC m_texture_init_desc{};
};

_NAMESPACE