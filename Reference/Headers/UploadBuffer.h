#pragma once
// Luna DX12 책 참고
#include "Base.h"
#include "Device_Utils.h"

NAMESPACE_(Engine)

// CPU 버퍼
template<typename T>
class CUploadBuffer : public CBase
{
	NO_COPY(CUploadBuffer)
private:
	CUploadBuffer(ID3D12Device* pDevice, UINT iElementCount, bool isConstantBuffer)
	{
		m_iElementByteSize = sizeof(T);

		// C-Buffer의 경우 GPU가 256 단위 오프셋에서 시작하는 256단위의  데이터만 볼 수 있음
		// C-buffer일 때만 256 단위로 조절
		if (isConstantBuffer)
		{
			m_iElementByteSize = CDevice_Utils::ConstantBufferByteSize(sizeof(T));
		}

		CD3DX12_HEAP_PROPERTIES tempHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		CD3DX12_RESOURCE_DESC tempResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(m_iElementByteSize * iElementCount);

		// Upload Buffer Resource 만들기
		if (FAILED(pDevice->CreateCommittedResource(
			&tempHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&tempResourceDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_pUploadBuffer))))
		{
			MSG_BOX("UploadBuffer Error");
			return;
		}

		// Map(CBuffer는 유일한 부분자원이라 인덱스 0, nullptr면 자원 전체, 자료)
		if (FAILED(m_pUploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&m_byMappedDataArr))))
		{
			MSG_BOX("UploadBuffer Error");
			return;
		} // 복사 후 원래는 UnMap 호출 후 메모리 해제 해야 함

	}
	~CUploadBuffer() override = default;


public:
	static CUploadBuffer* Create(ID3D12Device* pDevice, UINT iElementCount, bool isConstantBuffer)
	{
		return new CUploadBuffer(pDevice, iElementCount, isConstantBuffer);
	}
	HRESULT Free() override
	{
		if (m_pUploadBuffer != nullptr)
			m_pUploadBuffer->Unmap(0, nullptr);

		Safe_Release(m_pUploadBuffer);

		m_byMappedDataArr = nullptr;

		return S_OK;
	}
public:
	ID3D12Resource* Get_UploadBuffer() const { return m_pUploadBuffer; }
	void CopyData(_int iElementIndex, const T& data)
	{
		memcpy(&m_byMappedDataArr[iElementIndex * m_iElementByteSize], &data, sizeof(T));
	}
private:
	ID3D12Resource* m_pUploadBuffer = nullptr;
	BYTE* m_byMappedDataArr = nullptr;
	UINT m_iElementByteSize = 0; // C-buffer면 256단위
	bool m_IsConstantBuffer = false;
};

_NAMESPACE