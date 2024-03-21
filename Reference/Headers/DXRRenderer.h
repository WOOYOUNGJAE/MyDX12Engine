#pragma once
#include "Base.h"
// DirectX Raytracing Renderer

NAMESPACE_(Engine)
class CMeshData;
class CDXRRenderer : public CBase
{
protected:
	CDXRRenderer() = default;
	~CDXRRenderer() override = default;

public:
	static CDXRRenderer* Create();
	HRESULT Initialize(ID3D12Device** _Inout_ ppDevice);
	HRESULT Free() override;

	HRESULT Crete_RootSignatures();
	HRESULT Create_PSOs();
	HRESULT Build_AccelerationStructures();
private:
	ID3D12GraphicsCommandList4* m_pDxrCommandList = nullptr; // CmdList for Dxr
	ID3D12CommandAllocator* m_pCommandAllocator = nullptr;
	ID3D12RootSignature* m_pRootSigArr[DXR_ROOTSIG_TYPE_END];
private: // pointer
	ID3D12Device5* m_pDevice = nullptr;
	ID3D12StateObject* m_pDXR_PSO = nullptr;
private: // Manage
	std::map<CMeshData*, DXR::ACCELERATION_STRUCTURE_CPU> m_mapAS_CPU;
private: // entry point str
	static const wchar_t* m_tszHitGroupName;
	static const wchar_t* m_tszRaygenShaderName;
	static const wchar_t* m_tszClosestHitShaderName;
	static const wchar_t* m_tszMissShaderName;
};

_NAMESPACE