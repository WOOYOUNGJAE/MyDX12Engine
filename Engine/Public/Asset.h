#pragma once
#include "Base.h"

NAMESPACE_(Engine)

class ENGINE_DLL CAsset abstract : public CBase
{
	NO_COPY(CAsset)
protected:
	CAsset() = default;
	~CAsset() override = default;

public:
	virtual HRESULT Initialize(void* pArg = nullptr) = 0;
	HRESULT Free() override;

public:
	UINT64 m_iCbvSrvUavHeapOffset = 0;
protected:
	ID3D12Resource* m_pAssetData = nullptr;
	wstring m_strPath;
};

_NAMESPACE