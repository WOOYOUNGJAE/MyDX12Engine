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

private: // typedef
	typedef ComPtr<ID3D12PipelineState> PSO;
	enum ENUM_PSO { PSO_DEFAULT, ENUM_PSO_END };
private:
	class CGraphic_Device* m_pGraphic_Device = nullptr; // Singleton Class
	ComPtr<ID3D12Device> m_pDevice = nullptr; // Real Device
	ComPtr<ID3D12GraphicsCommandList> m_pCommandList = nullptr;
private:
	// ConstantBuffer
	ComPtr<ID3D12DescriptorHeap> m_pCbvHeap = nullptr;
	CUploadBuffer<_float4x4>* m_pUploadBuffer_Constant = nullptr;
private:
	PSO m_PSOArr[ENUM_PSO_END];
};

_NAMESPACE