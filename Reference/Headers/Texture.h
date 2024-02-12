#pragma once
#include "Asset.h"

NAMESPACE_(Engine)

class CTexture : public CAsset
{
	CTexture() = default;
	~CTexture() override = default;

public:
	static CTexture* Create(void* pArg);
	HRESULT Initialize(void* pArg) override;
private:
	bool m_bIsCubeMap;
};

_NAMESPACE