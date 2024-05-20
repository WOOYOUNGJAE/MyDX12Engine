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
#if DXR_ON
		pPassCB_DXR_scene = CUploadBuffer<DXR::SCENE_CB>::Create(pDevice, iPassCount, true);
		pObjectCB_Dynamic_DXR = CUploadBuffer<DXR::OBJECT_CB_DYNAMIC>::Create(pDevice, iObjectCount, true);
		pObjectCB_Static_DXR = CUploadBuffer<DXR::OBJECT_CB_STATIC>::Create(pDevice, iObjectCount, true);

#endif
	}
	~FrameResource()
	{
#if DXR_ON
		Safe_Release(pObjectCB_Static_DXR);
		Safe_Release(pObjectCB_Dynamic_DXR);
		Safe_Release(pPassCB_DXR_scene);
#endif
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
	// Command ������ �ؾ� �ִ��� üũ
	UINT64 Fence = 0;

#if DXR_ON
	CUploadBuffer<DXR::SCENE_CB>* pPassCB_DXR_scene = nullptr;
	CUploadBuffer<DXR::OBJECT_CB_DYNAMIC>* pObjectCB_Dynamic_DXR = nullptr;
	CUploadBuffer<DXR::OBJECT_CB_STATIC>* pObjectCB_Static_DXR = nullptr;
#endif
};

_NAMESPACE