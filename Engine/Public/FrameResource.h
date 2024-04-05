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

	// GPU�� ����� �� ó������ �Ҵ��ڸ� �缳���ؾ� �ϱ� ������ �����Ӹ��� �Ҵ��� �ʿ�
	ID3D12CommandAllocator* pCmdListAlloc;

	// ��� ���۴� GPU�� ��� �� ó���� �� �����ؾ� �ؼ� �� ������ ���ο� ������� �ʿ�
	CUploadBuffer<OBJECT_CB>* pObjectCB = nullptr;
	CUploadBuffer<PASS_CB_VP>* pPassCB = nullptr;
	CUploadBuffer<PASS_CB_VP_LIGHT>* pPassCB_vp_light = nullptr;
	//std::unique_ptr<UploadBuffer<PassConstants>> PassCB = nullptr;

	// Command ������ �ؾ� �ִ��� üũ
	UINT64 Fence = 0;
};

_NAMESPACE