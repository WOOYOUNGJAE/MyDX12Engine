#pragma once
#include "Base.h"

NAMESPACE_(Engine)

class CLoadHelper : public CBase
{
	DECLARE_SINGLETON(CLoadHelper)

public:
	CLoadHelper() = default;
	~CLoadHelper() override = default;

public:
	HRESULT Initialize();
	void LoadTextures_Begin();
	HRESULT LoadTextures();
	HRESULT Free() override;

private:
	ID3D12Device* m_pDevice = nullptr;
	class CComponentManager* m_pComponentManager = nullptr;
	class ResourceUploadBatch* m_pResourceUpload = nullptr;
};

_NAMESPACE