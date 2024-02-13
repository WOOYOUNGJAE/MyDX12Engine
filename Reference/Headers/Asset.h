#pragma once
#include "Base.h"

NAMESPACE_(Engine)
// ID3D12Resource와 오프셋 정보 저장
// ID3D12Resource생성 후 D3D12ResourceManager에 등록 해야
class ENGINE_DLL CAsset abstract : public CBase
{
	NO_COPY(CAsset)
protected:
	CAsset() = default;
	~CAsset() override = default;

public:
	virtual HRESULT Initialize(void* pArg = nullptr) = 0; // ID3D12Resource생성 후 D3D12ResourceManager에 등록 해야
	HRESULT Free() override;

public:
	UINT64 m_iCbvSrvUavHeapOffset = 0;
protected:
	ID3D12Resource* m_pAssetData = nullptr;
	wstring m_strPath;
};

_NAMESPACE