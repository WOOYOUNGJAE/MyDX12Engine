#include "UploadBuffer.h"

NAMESPACE_(Engine)

struct FrameResource
{
	NO_COPY(FrameResource);
	FrameResource(ID3D12Device* pDevice, UINT iObjectCount, UINT iPassCount = 0)
	{
		pDevice->CreateCommandAllocator(
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			IID_PPV_ARGS(&pCmdListAlloc));

		//PassCB = std::make_unique<UploadBuffer<PassConstants>>(device, passCount, true);
		pObjectCB = CUploadBuffer<OBJECT_CB>::Create(pDevice, iObjectCount, true);
		pPassCB = CUploadBuffer<PASS_CB_VP>::Create(pDevice, iPassCount, true);
		pPassCB_vp_light = CUploadBuffer<PASS_CB_VP_LIGHT>::Create(pDevice, iPassCount, true);
	}
	~FrameResource()
	{
		Safe_Release(pPassCB_vp_light);
		Safe_Release(pPassCB);
		Safe_Release(pObjectCB);
		Safe_Release(pCmdListAlloc);
	}

	// GPU가 명령을 다 처리한후 할당자를 재설정해야 하기 때문에 프레임마다 할당자 필요
	ID3D12CommandAllocator* pCmdListAlloc;

	// 상수 버퍼는 GPU가 명령 다 처리한 후 갱신해야 해서 매 프레임 새로운 상수버퍼 필요
	CUploadBuffer<OBJECT_CB>* pObjectCB = nullptr;
	CUploadBuffer<PASS_CB_VP>* pPassCB = nullptr;
	CUploadBuffer<PASS_CB_VP_LIGHT>* pPassCB_vp_light = nullptr;
	//std::unique_ptr<UploadBuffer<PassConstants>> PassCB = nullptr;

	// Command 어디까지 해야 있는지 체크
	UINT64 Fence = 0;
};

_NAMESPACE