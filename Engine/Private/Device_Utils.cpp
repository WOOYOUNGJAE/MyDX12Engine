#include "Device_Utils.h"

HRESULT CDevice_Utils::Create_Buffer_Default(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList,
	const void* initData, UINT64 byteSize, Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer,
	Microsoft::WRL::ComPtr<ID3D12Resource>* pOutResource)
{
    ComPtr<ID3D12Resource> defaultBuffer;

    CD3DX12_HEAP_PROPERTIES temp_heap_properties(D3D12_HEAP_TYPE_DEFAULT);
    CD3DX12_RESOURCE_DESC TempBuffer =
        CD3DX12_RESOURCE_DESC::Buffer (byteSize);

    // �⺻ ���� �ڿ� ����
    if (FAILED(device->CreateCommittedResource(
        &temp_heap_properties,
        D3D12_HEAP_FLAG_NONE,
        &TempBuffer,
        D3D12_RESOURCE_STATE_COMMON,
        nullptr,
        IID_PPV_ARGS(defaultBuffer.GetAddressOf()))))
    {
        return E_FAIL;
    }

    // GPU�� ���ۿ� ���翡 �غ� �ʿ��� �ӽ� ���ε� �� ����
    temp_heap_properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    if (FAILED(device->CreateCommittedResource(
        &temp_heap_properties,
        D3D12_HEAP_FLAG_NONE,
        &TempBuffer,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(uploadBuffer.GetAddressOf()))))
    {
        return E_FAIL;
    }

    // �⺻ ���ۿ� ������ �ڷ� ����
    D3D12_SUBRESOURCE_DATA subResourceData = {};
    subResourceData.pData = initData;
    // ���ۿ� �����ؼ� ������ �ڷ��� ũ�� (����Ʈ ����)�� ����
    subResourceData.RowPitch = byteSize;
    subResourceData.SlicePitch = subResourceData.RowPitch;

    // �⺻ ���� �ڿ������� �ڷ� ���� ��û
    // UpdateSubresource�� �ӽ� ���ε� ���� ������ �ø�
    // ID3D12CommandList::CopySubresourceRegion�� �ӽ� ���ε� ���� �ڷḦ buffer�� ����
    CD3DX12_RESOURCE_BARRIER tempBarrier = CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(),
        D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
    cmdList->ResourceBarrier(1, &tempBarrier);

    tempBarrier = CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(), // TODO
        D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
    UpdateSubresources<1>(cmdList, defaultBuffer.Get(), uploadBuffer.Get(), 0, 0, 1, &subResourceData);
    cmdList->ResourceBarrier(1, &tempBarrier);

    *pOutResource = defaultBuffer; // 

    // �� �Լ� ȣ�� ���Ŀ��� uploadBuffer�� �����ؾ� ��
    // ���簡 �Ϸ�Ǿ����� Ȯ������ �Ŀ� �����ϸ� ��

	return S_OK;
}
