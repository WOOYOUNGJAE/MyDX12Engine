#pragma once
#include "Base.h"

#if DXR_ON

NAMESPACE_(Engine)
class CMeshData;
using namespace std;
class CDXRResource : public CBase
{
	DECLARE_SINGLETON(CDXRResource)
	friend class CXRRenderer;

protected:
	CDXRResource() = default;
	~CDXRResource() override = default;

public:
	HRESULT Initialize();
	HRESULT Free() override;
public: // getter
	CD3DX12_CPU_DESCRIPTOR_HANDLE& Get_refHeapHandle_CPU() { return m_curHeapHandle_CPU; }
	UINT Get_DescriptorSize() { return m_iDescriptorSize; }
	ID3D12Resource*& Get_ScratchBufferRef() { return m_pScratchBuffer; }
	ID3D12Resource** Get_ScratchBufferPtr() { return &m_pScratchBuffer; }
public:
	HRESULT Crete_RootSignatures();
	HRESULT Create_PSOs();
	HRESULT Build_AccelerationStructures_CPU();
	HRESULT Build_AccelerationStructure_GPU(DXR::ACCELERATION_STRUCTURE_CPU& refAS_CPU);
	HRESULT Reset_CommandList();
	HRESULT Close_CommandList();
	HRESULT Execute_CommnadList();
	static void BuildRaytracingAccelerationStructure(
		_In_  const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC* pDesc,
		_In_  UINT NumPostbuildInfoDescs,
		_In_reads_opt_(NumPostbuildInfoDescs)  const D3D12_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO_DESC* pPostbuildInfoDescs)
	{
		Get_Instance()->m_pCommandList->BuildRaytracingAccelerationStructure(pDesc, NumPostbuildInfoDescs, pPostbuildInfoDescs);
	}
	void Flush_CommandQueue();
private: // D3D Resource
	ID3D12GraphicsCommandList4* m_pCommandList = nullptr; // CmdList for Dxr
	ID3D12CommandAllocator* m_pCommandAllocator = nullptr;
	ID3D12DescriptorHeap* m_pDescriptorHeap = nullptr; // CbvSrvUav Heap
	ID3D12RootSignature* m_pRootSigArr[DXR_ROOTSIG_TYPE_END];
	ID3D12StateObject* m_pDXR_PSO = nullptr;
	ID3D12Resource* m_pScratchBuffer = nullptr; // AS 빌드 중 필요한 낙서 버퍼.
private: // pointer
	ID3D12Device5* m_pDevice = nullptr;
	ID3D12CommandQueue* m_pCommandQueue = nullptr; // CommandQueue는 기존거와 공유
private:
	UINT m_iDescriptorSize = 0;
	CD3DX12_CPU_DESCRIPTOR_HANDLE m_curHeapHandle_CPU; // CbvSrvUav Handle Cpu
private: //Fence
	ID3D12Fence* m_pFence = nullptr;
	UINT64 m_iFenceValue = 0;
	HANDLE m_fenceEvent;
	QUEUE_FLUSH_DESC m_queue_flush_desc{};
private: // Manage
	/*map<CMeshData*, DXR::ACCELERATION_STRUCTURE_CPU> m_mapAS_CPU;
	map<CMeshData*, D3D12_RAYTRACING_GEOMETRY_DESC> m_mapDXRGeometryDesc;*/
private: // entry point str
	static const wchar_t* m_tszHitGroupName;
	static const wchar_t* m_tszRaygenShaderName;
	static const wchar_t* m_tszClosestHitShaderName;
	static const wchar_t* m_tszMissShaderName;
};

_NAMESPACE

#endif