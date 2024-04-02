#pragma once
#include "Base.h"

#if DXR_ON

NAMESPACE_(Engine)
using DXR::TABLE_RECORD_DESC;

class CDXRShaderTable : public CBase
{
protected:
	CDXRShaderTable() = default;
	~CDXRShaderTable() override = default;

public:
	static CDXRShaderTable* Create(ID3D12Device* pDevice, UINT iNumShaderRecords, UINT iRecordSize, LPCWSTR resourceName = nullptr);
	HRESULT Initialize(ID3D12Device* pDevice, UINT iNumShaderRecords, UINT iRecordSize, LPCWSTR resourceName = nullptr);
	HRESULT Free() override; // TableData(Resource)는 외부에서 함
public:
	ID3D12Resource* Get_TableResource() { return m_pTableData; }
public:
	void Register_Record(const TABLE_RECORD_DESC& inRecord);

private:
	ID3D12Resource* m_pTableData = nullptr;
private:
	BYTE* m_pRecordingData = nullptr; // mapping하는 포인터
	UINT m_iSingleRecordSize = 0;
	std::wstring m_strName;
	std::vector<TABLE_RECORD_DESC> m_vecRecords;

};

_NAMESPACE

#endif
