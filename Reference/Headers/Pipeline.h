#pragma once
#include "Base.h"
#include "UploadBuffer.h"

NAMESPACE_(Engine)
using namespace std;
// 파이프라인 중 공유해야 하는 자원 관리
// CGraphic_Device's Friend
class CPipeline : public CBase
{
	DECLARE_SINGLETON(CPipeline)
private:
	CPipeline();
	~CPipeline() override = default;

public: // typedef
	typedef ComPtr<ID3D12PipelineState> PSO;
	typedef list<class CGameObject*> PipelineLayer;
	enum ENUM_PSO { PSO_DEFAULT, PSO_END };
	enum ENUM_RootSig {RootSig_DEFAULT, RootSig_END};

public:
	HRESULT Initialize();
	HRESULT Init_ConstantBuffers();
	HRESULT Init_RootSignature();
	void Pipeline_Tick();
	HRESULT Free() override;
public:
	CUploadBuffer<_float4x4>* Get_UploadBuffer() { return m_pUploadBuffer_Constant; }
	// TODO : Build PSO Overrides
	HRESULT Build_PSO(const D3D12_GRAPHICS_PIPELINE_STATE_DESC& pipeline_desc, ENUM_PSO psoIndex);

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
	PipelineLayer m_vecPipelineLayerArr[PSO_END]; // 게임 오브젝트의 Pipeline_Tick을 대신해 돌려주는 함수
	//CD3DX12_PIPELINE_STATE_STREAM_SUBOBJECT<>
};

_NAMESPACE