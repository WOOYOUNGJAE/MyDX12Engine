#pragma once
#include "Base.h"
// ID3D12Resource ����
// Resource�� ������ �� ���۷��� ī��Ʈ ������Ų �� �Ŵ����� ���
// ���� ��� : Texture, ConstantBuffer

NAMESPACE_(Engine)

class ENGINE_DLL CD3DResourceManager : public CBase
{
	DECLARE_SINGLETON(CD3DResourceManager)
	CD3DResourceManager() = default;
	~CD3DResourceManager() override = default;

public:
	void Register_Resource(UINT eType, ID3D12Resource** ppResource);
	void Initialize();
	HRESULT Free() override;
public: // getter Setter
	ID3D12Resource* Get_Resource(UINT eType, UINT iRegisterIndex);
	UINT64 Get_SrvOffsetStart() { return m_iSrvOffsetStart; }
	UINT64 Get_LastSrvOffset() { return m_iLastSrvOffset; }
	UINT64 Get_SrvOffsetEnd() { return m_iSrvOffsetEnd; }
	UINT Get_CbvSrvUavDescriptorSize() { return m_iCbvSrvUavDescriptorSize; }
	void Set_SrvOffsetStart(UINT64 iOffset) { m_iSrvOffsetStart = iOffset; }
	void Set_LastSrvOffset(UINT64 iOffset) { m_iLastSrvOffset = iOffset; }
	void Set_SrvOffsetEnd(UINT64 iOffset)
	{
		m_iSrvOffsetEnd = iOffset;
		m_iLastSrvOffset = m_iSrvOffsetEnd - m_iCbvSrvUavDescriptorSize;
	}
private:
	UINT m_iCurRegisterIndexArr[MANAGED_RESOURCE_TYPE_END]; // ����� �� �ε��� ����, 0���� ���� Ǫ���� ������ ����
	std::vector<ID3D12Resource*> m_vecID3D12ResourceArr[MANAGED_RESOURCE_TYPE_END]; // Ÿ�Ը����� ���ҽ� ����
private: // Offset
	UINT m_iCbvSrvUavDescriptorSize = 0;
	UINT64 m_iSrvOffsetStart = 0;
	UINT64 m_iLastSrvOffset = 0;
	UINT64 m_iSrvOffsetEnd = 0;

};

_NAMESPACE