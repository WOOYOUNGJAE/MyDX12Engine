#pragma once
#include "Base.h"
#include "UploadBuffer.h"

NAMESPACE_(Engine)

// 파이프라인 중 공유해야 하는 자원 관리
// CGraphic_Device's Friend
class CPipeline : public CBase
{
	DECLARE_SINGLETON(CPipeline)
private:
	CPipeline();
	~CPipeline() override = default;

public:
	HRESULT Initialize();
	HRESULT Init_ConstantBuffers();
	HRESULT Init_RootSignature();
	HRESULT Free() override;
public:
	CUploadBuffer<_float4x4>* Get_UploadBuffer() { return m_pUploadBuffer_Constant; }

public: // typedef
	typedef ComPtr<ID3D12PipelineState> PSO;
	enum ENUM_PSO { PSO_DEFAULT, PSO_END };
	enum ENUM_RootSig {RootSig_DEFAULT, RootSig_END};
private: // Graphic Device
	class CGraphic_Device* m_pGraphic_Device = nullptr; // Singleton Class
	ComPtr<ID3D12Device> m_pDevice = nullptr; // Real Device
	ComPtr<ID3D12GraphicsCommandList> m_pCommandList = nullptr;
private: // ConstantBuffer	
	ComPtr<ID3D12DescriptorHeap> m_pCbvHeap = nullptr;
	CUploadBuffer<_float4x4>* m_pUploadBuffer_Constant = nullptr;
private: // Root Signature
	ComPtr<ID3D12RootSignature> m_RootSig[RootSig_END];
private: // PSO
	PSO m_PSOArr[PSO_END];
};

_NAMESPACE