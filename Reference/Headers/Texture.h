#pragma once
#include "Component.h"

NAMESPACE_(Engine)

class ENGINE_DLL CTexture : public CComponent
{
private:
	CTexture();
	CTexture(const CTexture& rhs);
	~CTexture() override = default;

public:
	static CTexture* Create(void* pArg);
	CComponent* Clone(void* pArg) override;
	HRESULT Initialize_Prototype();
	HRESULT Initialize_Prototype(TEXTURE_INIT_DESC* pInitDesc);
	HRESULT Initialize(void* pArg) override;
	HRESULT Free() override;

public: // for Prototype
	UINT64 m_iCbvSrvUavHeapOffset = 0;
private:
	ID3D12Resource* m_pTextureData = nullptr;
	wstring m_strPath;
	bool m_bIsCubeMap;
};

_NAMESPACE
