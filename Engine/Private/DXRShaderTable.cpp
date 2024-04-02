#include "DXRShaderTable.h"

#include "Device_Utils.h"

CDXRShaderTable* CDXRShaderTable::Create(ID3D12Device* pDevice, UINT iNumShaderRecords, UINT iRecordSize,
                                         LPCWSTR resourceName)
{
	CDXRShaderTable* pInstnace = new CDXRShaderTable;

	if (FAILED(pInstnace->Initialize(pDevice, iNumShaderRecords, iRecordSize, resourceName)))
	{
		Safe_Release(pInstnace);
	}

	return pInstnace;
}

HRESULT CDXRShaderTable::Initialize(ID3D12Device* pDevice, UINT iNumShaderRecords, UINT iRecordSize,
                                    LPCWSTR resourceName)
{
	HRESULT hr = S_OK;

	m_iSingleRecordSize = MyUtils::Align(iRecordSize, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT);
	m_vecRecords.reserve(iNumShaderRecords);
	UINT iBufferSize = iNumShaderRecords * m_iSingleRecordSize;

	hr = MyUtils::AllocateUploadBuffer(pDevice, nullptr, iBufferSize, &m_pTableData, resourceName);
	if (FAILED(hr)) { return hr; }

	CD3DX12_RANGE readRange(0, 0);
	// 최종 해제될 때 UnMap
	hr = m_pTableData->Map(0, &readRange, reinterpret_cast<void**>(&m_pRecordingData));
	if (FAILED(hr)) { return hr; }
	

	return hr;
}

HRESULT CDXRShaderTable::Free()
{
	m_pTableData->Unmap(0, nullptr);

	return S_OK;
}

void CDXRShaderTable::Register_Record(const TABLE_RECORD_DESC& inRecord)
{
	m_vecRecords.emplace_back(inRecord);

	memcpy(m_pRecordingData, inRecord.pIdentifierData, inRecord.iIdentifierSize);
	
	if (inRecord.pLocalRootArgumentData)
	{
		memcpy(m_pRecordingData + inRecord.iIdentifierSize, inRecord.pLocalRootArgumentData, inRecord.iLocalRootArgumentSize);
	}

	m_pRecordingData += m_iSingleRecordSize;
}
