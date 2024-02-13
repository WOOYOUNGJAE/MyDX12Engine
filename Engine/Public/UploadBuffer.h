#pragma once
// Luna DX12 å ����
#include "Base.h"
#include "Device_Utils.h"

NAMESPACE_(Engine)

// CPU ����
template<typename T>
class CUploadBuffer : public CBase
{
	NO_COPY(CUploadBuffer)
private:
	CUploadBuffer(ID3D12Device* pDevice, UINT iElementCount, bool isConstantBuffer)
	{
		m_iElementByteSize = sizeof(T);

		// C-Buffer�� ��� GPU�� 256 ���� �����¿��� �����ϴ� 256������  �����͸� �� �� ����
		// C-buffer�� ���� 256 ������ ����
		if (isConstantBuffer)
		{
			m_iElementByteSize = CDevice_Utils::ConstantBufferByteSize(sizeof(T));
		}

		CD3DX12_HEAP_PROPERTIES tempHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		CD3DX12_RESOURCE_DESC tempResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(m_iElementByteSize * iElementCount);

		// Upload Buffer Resource �����
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

		// Map(CBuffer�� ������ �κ��ڿ��̶� �ε��� 0, nullptr�� �ڿ� ��ü, �ڷ�)
		if (FAILED(m_pUploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&m_byMappedDataArr))))
		{
			MSG_BOX("UploadBuffer Error");
			return;
		} // ���� �� ������ UnMap ȣ�� �� �޸� ���� �ؾ� ��

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
	UINT m_iElementByteSize = 0; // C-buffer�� 256����
	bool m_IsConstantBuffer = false;
};

_NAMESPACE