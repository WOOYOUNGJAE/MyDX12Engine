#include "FrameResourceManager.h"
#include "FrameResource.h"
#include "DeviceResource.h"
#include "Renderer.h"
#include "DXRRenderer.h"

IMPLEMENT_SINGLETON(CFrameResourceManager)

HRESULT CFrameResourceManager::Initialize()
{
	HRESULT hr = S_OK;
	CDeviceResource* pDeviceResource = CDeviceResource::Get_Instance();
	ID3D12Device* pDevice = pDeviceResource->m_pDevice.Get();
	m_iObjCbvDescriptorSize = pDeviceResource->Get_CbvSrvUavDescriptorSize();

	// FrameResource
	for (UINT i = 0; i < g_iNumFrameResource; ++i)
	{
		m_vecFrameResource.push_back(new FrameResource(
			pDevice,
			NUM_OBJECTS/**/,
			1/**/)
		);
	}
	m_pCurFrameResource = m_vecFrameResource[0]; // TODO FrameResource 2이상되면 수정
	return hr;
}

HRESULT CFrameResourceManager::Build_FrameResource()
{
	HRESULT hr = S_OK;
	CDeviceResource* pDeviceResource = CDeviceResource::Get_Instance();
	ID3D12Device* pDevice = pDeviceResource->m_pDevice.Get();
	

	// Build Obj Constant Buffer
	UINT objCBByteSize = MyUtils::Align256(sizeof(OBJECT_CB));
	UINT objCount = 1; //
	UINT iCbvSrvUavDescriptorSize = pDeviceResource->m_iCbvSrvUavDescriptorSize;
	auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(pDeviceResource->Get_CbvSrvUavHeapStart_CPU());
	m_iObjCBVHeapStartOffset = *pDeviceResource->Get_NextCbvSrvUavHeapOffsetPtr();
	UINT iTempPassCBVHeapStartOffset = m_iObjCBVHeapStartOffset;
	handle.Offset((INT)m_iObjCBVHeapStartOffset); // SRV 생성 후 Cbv 힙 시작 오프셋
	for (UINT iFrameIndex = 0; iFrameIndex < g_iNumFrameResource; ++iFrameIndex)
	{
		ID3D12Resource* pObjCB = m_vecFrameResource[iFrameIndex]->pObjectCB->Get_UploadBuffer();
		for (UINT i = 0; i < objCount; ++i)
		{
			D3D12_GPU_VIRTUAL_ADDRESS cbAddress = pObjCB->GetGPUVirtualAddress();

			// 현재 버퍼에서 i번째 물체별 상수 버퍼의 오프셋
			cbAddress += i * objCBByteSize;

			// 서술자 힙에서 i번째 물체별 상수 버퍼의 오프셋

			D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
			cbvDesc.BufferLocation = cbAddress;
			cbvDesc.SizeInBytes = objCBByteSize;

			pDevice->CreateConstantBufferView(&cbvDesc, handle);

			INT heapIndex = iFrameIndex * objCount + i;
			handle.Offset(heapIndex, iCbvSrvUavDescriptorSize);
			iTempPassCBVHeapStartOffset += iCbvSrvUavDescriptorSize; // 마지막 ObjCB 생성될 때까지 업데이트
		}
	}

	UINT passCBByteSize = MyUtils::Align256(sizeof(PASS_CB_VP));
	handle.InitOffsetted(pDeviceResource->Get_CbvSrvUavHeapStart_CPU(), 0);
	m_iPassCBVHeapStartOffset = iTempPassCBVHeapStartOffset;
	handle.Offset(m_iPassCBVHeapStartOffset);


	for (INT frameIndex = 0; frameIndex < g_iNumFrameResource; ++frameIndex)
	{
		passCBByteSize = MyUtils::Align256(sizeof(PASS_CB_VP_LIGHT));
		ID3D12Resource* passCB = m_vecFrameResource[frameIndex]->pPassCB_vp_light->Get_UploadBuffer();
		D3D12_GPU_VIRTUAL_ADDRESS cbAddress = passCB->GetGPUVirtualAddress();

		// Offset to the pass cbv in the descriptor heap.
		// 서술자 힙 안에서 Pass CBV의 오프셋
		INT heapIndex = m_iPassCBVHeapStartOffset + frameIndex;

		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
		cbvDesc.BufferLocation = cbAddress;
		cbvDesc.SizeInBytes = passCBByteSize;
		pDevice->CreateConstantBufferView(&cbvDesc, handle);

		handle.Offset(heapIndex, iCbvSrvUavDescriptorSize);
	}

	return hr;
}

HRESULT CFrameResourceManager::Free()
{
	// FrameResource
	for (auto& iter : m_vecFrameResource)
	{
		Safe_Delete(iter);
	}
	m_vecFrameResource.clear();

	return S_OK;
}

