#include "Device_Utils.h"

HRESULT CDevice_Utils::Create_Buffer_Default(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList,
	const void* initData, UINT64 byteSize, Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer,
	Microsoft::WRL::ComPtr<ID3D12Resource>* pOutResource)
{
    ComPtr<ID3D12Resource> defaultBuffer;

    CD3DX12_HEAP_PROPERTIES temp_heap_properties(D3D12_HEAP_TYPE_DEFAULT);
    CD3DX12_RESOURCE_DESC TempBuffer =
        CD3DX12_RESOURCE_DESC::Buffer (byteSize);

    // 기본 버퍼 자원 생성
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

    // GPU의 버퍼에 복사에 준비에 필요한 임시 업로드 힙 생성
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

    // 기본 버퍼에 복사할 자료 서술
    D3D12_SUBRESOURCE_DATA subResourceData = {};
    subResourceData.pData = initData;
    // 버퍼에 한정해서 복사할 자료의 크기 (바이트 개수)를 뜻함
    subResourceData.RowPitch = byteSize;
    subResourceData.SlicePitch = subResourceData.RowPitch;

    // 기본 버퍼 자원으로의 자료 복사 요청
    // UpdateSubresource로 임시 업로드 힙에 데이터 올림
    // ID3D12CommandList::CopySubresourceRegion로 임시 업로드 힙의 자료를 buffer에 복사
    CD3DX12_RESOURCE_BARRIER tempBarrier = CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(),
        D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
    cmdList->ResourceBarrier(1, &tempBarrier);

    tempBarrier = CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(), // TODO
        D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
    UpdateSubresources<1>(cmdList, defaultBuffer.Get(), uploadBuffer.Get(), 0, 0, 1, &subResourceData);
    cmdList->ResourceBarrier(1, &tempBarrier);

    *pOutResource = defaultBuffer; // 

    // 이 함수 호출 이후에도 uploadBuffer를 유지해야 함
    // 복사가 완료되었음이 확실해진 후에 해제하면 됨

	return S_OK;
}
