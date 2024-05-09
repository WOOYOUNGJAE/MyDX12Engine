#pragma once
#include "Base.h"

#if DXR_ON

NAMESPACE_(Engine)
class CMeshData;
using namespace std;
class CDXRResource : public CBase
{
	DECLARE_SINGLETON(CDXRResource)
	friend class CDXRRenderer;

protected:
	CDXRResource()
	{
		m_pCommandAllocatorArr = new ID3D12CommandAllocator * [m_iBackBufferCount]{};
	}
	~CDXRResource() override = default;

public:
	HRESULT Initialize();
	HRESULT Free() override;
public: // getter
	CD3DX12_CPU_DESCRIPTOR_HANDLE& Get_refHeapHandle_CPU() { return m_curHeapHandle_CPU; }
	D3D12_CPU_DESCRIPTOR_HANDLE Get_HeapHandleStart_CPU() { return m_pDescriptorHeap->GetCPUDescriptorHandleForHeapStart(); }
	D3D12_GPU_DESCRIPTOR_HANDLE Get_HeapHandleStart_GPU() { return m_pDescriptorHeap->GetGPUDescriptorHandleForHeapStart(); }
	UINT Get_DescriptorSize() { return m_iDescriptorSize; }
	ID3D12Resource*& Get_ScratchBufferRef() { return m_pScratchBuffer; }
	ID3D12Resource** Get_ScratchBufferPtr() { return &m_pScratchBuffer; }
	ID3D12CommandAllocator*** Get_CommandAllocatorArrPtr() { return &m_pCommandAllocatorArr; }
	ID3D12GraphicsCommandList4* Get_CommandList4() { return m_pCommandList; }
	ID3D12CommandQueue* Get_CommandQueue() { return m_pCommandQueue; }
private:
	HRESULT Crete_RootSignatures();
	HRESULT Create_PSOs();
	HRESULT Build_ShaderTable();
	HRESULT Create_OutputResource();
public:
	HRESULT Reset_CommandList();
	HRESULT Close_CommandList();
	HRESULT Execute_CommnadList();
	void AssignShaderIdentifiers(ID3D12StateObjectProperties* stateObjectProperties, void** ppRayGenShaderIdentifier, void** ppMissShaderIdentifier, void** ppHitGroupShaderIdentifier);
	static void BuildRaytracingAccelerationStructure(
		_In_  const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC* pDesc,
		_In_  UINT NumPostbuildInfoDescs,
		_In_reads_opt_(NumPostbuildInfoDescs)  const D3D12_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO_DESC* pPostbuildInfoDescs)
	{
		Get_Instance()->m_pCommandList->BuildRaytracingAccelerationStructure(pDesc, NumPostbuildInfoDescs, pPostbuildInfoDescs);
	}
	static void ResourceBarrierUAV(ID3D12Resource* pResource) { 1, & CD3DX12_RESOURCE_BARRIER::UAV(pResource); }
	void Flush_CommandQueue();
private: // D3D Resource
	ID3D12GraphicsCommandList4* m_pCommandList = nullptr; // CmdList for Dxr
	ID3D12CommandAllocator** m_pCommandAllocatorArr = nullptr;
	ID3D12DescriptorHeap* m_pDescriptorHeap = nullptr; // CbvSrvUav Heap
	ID3D12RootSignature* m_pRootSigArr[DXR_ROOTSIG_TYPE_END]{};
	ID3D12StateObject* m_pDXR_PSO = nullptr;
	ID3D12Resource* m_pScratchBuffer = nullptr; // AS 빌드 중 필요한 낙서 버퍼.
	ID3D12Resource* m_pDXROutput = nullptr;
private: // Shader Table
	ID3D12Resource* m_pRayGenShaderTable = nullptr;
	ID3D12Resource* m_pMissShaderTable = nullptr;
	ID3D12Resource* m_pHitGroupShaderTable = nullptr;
private: // pointer
	ID3D12Device5* m_pDevice = nullptr;
	ID3D12CommandQueue* m_pCommandQueue = nullptr; // CommandQueue는 기존거와 공유
private:
	UINT m_iDescriptorSize = 0;
	CD3DX12_CPU_DESCRIPTOR_HANDLE m_curHeapHandle_CPU; // CbvSrvUav Handle Cpu
	CD3DX12_GPU_DESCRIPTOR_HANDLE m_DXROutputHeapHandle;

private: //Fence
	ID3D12Fence* m_pFence = nullptr;
	UINT64 m_iFenceValue = 0;
	HANDLE m_fenceEvent;
	QUEUE_FLUSH_DESC m_queue_flush_desc{};
private:
	UINT m_iBackBufferCount = 2;
	UINT m_iScreenWidth = 0;
	UINT m_iScreenHeight = 0;
private: // entry point str
	static const wchar_t* m_tszHitGroupName;
	static const wchar_t* m_tszRaygenShaderName;
	static const wchar_t* m_tszClosestHitShaderName;
	static const wchar_t* m_tszMissShaderName;
};

_NAMESPACE

#endif