#pragma once
#include <unordered_map>

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
	enum ENUM_PIPELINE_MAT {VIEW_MAT, PROJ_MAT, ENUM_PIPELINE_MAT_END};
	enum ENUM_PSO { PSO_DEFAULT, PSO_END };
	enum ENUM_RootSig {RootSig_DEFAULT, RootSig_END};
	enum ENUM_InputLayout {InputLayout_DEFAULT, InputLayout_END	};
	struct FrameResource
	{
	public:
		NO_COPY(FrameResource);
		FrameResource(ID3D12Device* device, UINT passCount, UINT objectCount);
		~FrameResource() { Safe_Release(CmdListAlloc); };

		// GPU가 명령을 다 처리한후 할당자를 재설정해야 하기 때문에 프레임마다 할당자 필요
		ComPtr<ID3D12CommandAllocator> CmdListAlloc;

		// 상수 버퍼는 GPU가 명령 다 처리한 후 갱신해야 해서 매 프레임 새로운 상수버퍼 필요
		
		/*std::unique_ptr<UploadBuffer<PassConstants>> PassCB = nullptr;
		std::unique_ptr<UploadBuffer<ObjectConstants>> ObjectCB = nullptr;*/

		// Command 어디까지 해야 있는지 체크
		UINT64 Fence = 0;
	};
public:
	HRESULT Initialize();
	void Pipeline_Tick();
	HRESULT Free() override;
public:
	CUploadBuffer<_float4x4>* Get_UploadBuffer() { return m_pUploadBuffer_Constant; }
	// TODO : Build PSO Overrides
	HRESULT Build_PSO(const D3D12_GRAPHICS_PIPELINE_STATE_DESC& pipeline_desc, ENUM_PSO psoIndex);
	void Push_InputLayout(D3D12_INPUT_ELEMENT_DESC desc, ENUM_InputLayout eEnum)
	{
		m_vecInputLayout[eEnum].push_back(desc);
	}
public: // Cam, View
	/*_float4 Get_CamPos() { return m_CamPosition; }
	_float4x4 Get_CamMatrix() { return m_pipelineMatrix; }
	_float4x4 Get_CamMatrix_Inverse()
	{
		_float4x4 outPut;
		XMStoreFloat4x4(&outPut, XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_pipelineMatrix)));
		return outPut;
	}*/
	void Update_Matrix(_float4x4 Matrix, ENUM_PIPELINE_MAT eEnum) { m_pipelineMatrix[eEnum] = Matrix; }
	void Update_Matrix(_fmatrix mMatrix, ENUM_PIPELINE_MAT eEnum) { XMStoreFloat4x4(&m_pipelineMatrix[eEnum], mMatrix); }
private:
	HRESULT Init_FrameResource();
	HRESULT Init_ConstantBuffers();
	HRESULT Init_RootSignature();

private: // Graphic Device
	class CGraphic_Device* m_pGraphic_Device = nullptr; // Singleton Class
	ComPtr<ID3D12Device> m_pDevice = nullptr; // Real Device
	ComPtr<ID3D12GraphicsCommandList> m_pCommandList = nullptr;
private: // Frame Resource
	//
	vector<FrameResource*> m_vecFrameResource;
	//unordered_map<string, ComPtr<PassConstant>>  
	//unordered_map<string, ComPtr<ObjectConstant>>  
private: // Camera Property
	_float4x4 m_pipelineMatrix[ENUM_PIPELINE_MAT_END];
	//_float4 m_CamPosition;
private: // ConstantBuffer	
	ComPtr<ID3D12DescriptorHeap> m_pCbvHeap = nullptr;
	CUploadBuffer<_float4x4>* m_pUploadBuffer_Constant = nullptr;
private: // Input layout
	vector<D3D12_INPUT_ELEMENT_DESC> m_vecInputLayout[InputLayout_END];
private: // Root Signature
	ComPtr<ID3D12RootSignature> m_RootSig[RootSig_END];
private: //Shader Class
	map<wstring, class CShader*> m_map_Shader;
private: // PSO
	PSO m_PSOArr[PSO_END];
	PipelineLayer m_vecPipelineLayerArr[PSO_END]; // 게임 오브젝트의 Pipeline_Tick을 대신해 돌려주는 함수
	//CD3DX12_PIPELINE_STATE_STREAM_SUBOBJECT<>
private: // Pointer
	class CComponentManager* m_pComponentManager = nullptr;
};

_NAMESPACE