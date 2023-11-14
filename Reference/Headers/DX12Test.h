#pragma once
#include "Engine_Defines.h"
class DX12Test
{
	DX12Test()
	{
		D3D12_COMMAND_QUEUE_DESC command_desc;

		//ID3D12CommandQueue::ExecuteCommandLists();
		//ID3D12GraphicsCommandList::Close(); // ´Ý±â

        enum D3D12_COMMAND_LIST_TYPE
        {
            D3D12_COMMAND_LIST_TYPE_DIRECT = 0, // 
            D3D12_COMMAND_LIST_TYPE_BUNDLE = 1,
            D3D12_COMMAND_LIST_TYPE_COMPUTE = 2,
            D3D12_COMMAND_LIST_TYPE_COPY = 3,
            D3D12_COMMAND_LIST_TYPE_VIDEO_DECODE = 4,
            D3D12_COMMAND_LIST_TYPE_VIDEO_PROCESS = 5,
            D3D12_COMMAND_LIST_TYPE_VIDEO_ENCODE = 6
        } 	D3D12_COMMAND_LIST_TYPE;
	}
};

