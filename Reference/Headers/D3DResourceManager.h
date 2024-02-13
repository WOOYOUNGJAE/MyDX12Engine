#pragma once
#include "Base.h"
// ID3D12Resource ����
// Resource�� ������ �� ���۷��� ī��Ʈ ������Ų �� �Ŵ����� ���
// ���� ��� : Texture, ConstantBuffer

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
	UINT m_iCurRegisterIndexArr[MANAGED_RESOURCE_TYPE_END]; // ����� �� �ε��� ����, 0���� ���� Ǫ���� ������ ����
	std::vector<ID3D12Resource*> m_vecID3D12ResourceArr[MANAGED_RESOURCE_TYPE_END]; // Ÿ�Ը����� ���ҽ� ����
};

_NAMESPACE