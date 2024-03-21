#pragma once
#include "Base.h"

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
public:
	HRESULT Crete_RootSignatures();
	HRESULT Create_PSOs();
	HRESULT Build_AccelerationStructures();
private: // D3D Resource
	ID3D12GraphicsCommandList4* m_pCommandList = nullptr; // CmdList for Dxr
	ID3D12CommandAllocator* m_pCommandAllocator = nullptr;
	ID3D12DescriptorHeap* m_pDescriptorHeap = nullptr; // CbvSrvUav Heap
	ID3D12RootSignature* m_pRootSigArr[DXR_ROOTSIG_TYPE_END];
	ID3D12StateObject* m_pDXR_PSO = nullptr;
private: // pointer
	ID3D12Device5* m_pDevice = nullptr;
private:
	UINT m_iDescriptorSize = 0;
	UINT m_iCbvSrvUavOffset = 0;
private: // Manage
	map<CMeshData*, DXR::ACCELERATION_STRUCTURE_CPU> m_mapAS_CPU;
private: // entry point str
	static const wchar_t* m_tszHitGroupName;
	static const wchar_t* m_tszRaygenShaderName;
	static const wchar_t* m_tszClosestHitShaderName;
	static const wchar_t* m_tszMissShaderName;
};

_NAMESPACE