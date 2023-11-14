#pragma once
#include "Engine_Defines.h"
class DX12Test
{
    DX12Test()
    {
        D3D12_COMMAND_QUEUE_DESC command_desc;

        //ID3D12CommandQueue::ExecuteCommandLists();
        //ID3D12GraphicsCommandList::Close(); // 닫기

        enum D3D12_COMMAND_LIST_TYPE
        {
            D3D12_COMMAND_LIST_TYPE_DIRECT = 0, // 
            D3D12_COMMAND_LIST_TYPE_BUNDLE = 1,
            D3D12_COMMAND_LIST_TYPE_COMPUTE = 2,
            D3D12_COMMAND_LIST_TYPE_COPY = 3,
            D3D12_COMMAND_LIST_TYPE_VIDEO_DECODE = 4,
            D3D12_COMMAND_LIST_TYPE_VIDEO_PROCESS = 5,
            D3D12_COMMAND_LIST_TYPE_VIDEO_ENCODE = 6
        };

        typedef struct D3D12_INPUT_LAYOUT_DESC
        {
            const D3D12_INPUT_ELEMENT_DESC* pInputElementDescs; // 원소 배열
            UINT NumElements;
        } 	D3D12_INPUT_LAYOUT_DESC;

        typedef struct D3D12_INPUT_ELEMENT_DESC
        {
            LPCSTR SemanticName; // 사용자가 지정한 성분 이름
            UINT SemanticIndex; // 배열에서의 인덱스
            DXGI_FORMAT Format; // 성분의 형식
            UINT InputSlot;
            UINT AlignedByteOffset;
            D3D12_INPUT_CLASSIFICATION InputSlotClass; // 기본값 0
            UINT InstanceDataStepRate;// 기본값 0
        } 	D3D12_INPUT_ELEMENT_DESC;

        //ID3D12Device::CreateCommittedResource()
        D3D12_RESOURCE_DESC;
        CD3DX12_RESOURCE_DESC;
        D3D12_HEAP_TYPE;
    }
};

