#pragma once
#include "Base.h"
// ID3D12Resource 관리
// Resource가 생성될 때 레퍼런스 카운트 증가시킨 후 매니저에 등록
// 관리 대상 : Texture, ConstantBuffer

NAMESPACE_(Engine)

class CD3DResourceManager : public CBase
{
	DECLARE_SINGLETON(CD3DResourceManager)
	CD3DResourceManager() = default;
	~CD3DResourceManager() override = default;

public:
	void Register_Resource(UINT eType, ID3D12Resource** ppResource);
	HRESULT Free() override;
public: // getter
	ID3D12Resource* Get_Resource(UINT eType, UINT iRegisterIndex);

private:
	UINT m_iCurRegisterIndexArr[MANAGED_RESOURCE_TYPE_END]; // 등록할 때 인덱스 역할, 0부터 시작 푸쉬할 때마다 증가
	std::vector<ID3D12Resource*> m_vecID3D12ResourceArr[MANAGED_RESOURCE_TYPE_END]; // 타입마다의 리소스 벡터
};

_NAMESPACE