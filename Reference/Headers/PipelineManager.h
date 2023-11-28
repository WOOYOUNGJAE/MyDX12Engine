#pragma once
#include <unordered_map>

#include "Base.h"
#include "UploadBuffer.h"

NAMESPACE_(Engine)
using namespace std;
using namespace Pipeline;
// 파이프라인 중 공유해야 하는 자원 관리
// CGraphic_Device's Friend
class CPipelineManager : public CBase
{
	DECLARE_SINGLETON(CPipelineManager)
private:
	CPipelineManager();
	~CPipelineManager() override = default;

public: // typedef
	//typedef ComPtr<ID3D12PipelineState> PSO;
	typedef list<class CGameObject*> PipelineLayer;
	enum ENUM_PIPELINE_MAT {VIEW_MAT, PROJ_MAT, ENUM_PIPELINE_MAT_END};
	enum ENUM_RootSig {RootSig_DEFAULT, RootSig_END};
	enum ENUM_InputLayout {InputLayout_DEFAULT, InputLayout_END	};

	struct ObjectConstants
	{
		_float4x4 World = Matrix_Identity();
	};
	struct PassConstants
	{
		XMFLOAT4X4 View = Matrix_Identity();
		XMFLOAT4X4 InvView = Matrix_Identity();
		XMFLOAT4X4 Proj = Matrix_Identity();
		XMFLOAT4X4 InvProj = Matrix_Identity();
		XMFLOAT4X4 ViewProj = Matrix_Identity();
		XMFLOAT4X4 InvViewProj = Matrix_Identity();
		XMFLOAT3 EyePosW = { 0.0f, 0.0f, 0.0f };
		float cbPerObjectPad1 = 0.0f;
		XMFLOAT2 RenderTargetSize = { 0.0f, 0.0f };
		XMFLOAT2 InvRenderTargetSize = { 0.0f, 0.0f };
		float NearZ = 0.0f;
		float FarZ = 0.0f;
		float TotalTime = 0.0f;
		float DeltaTime = 0.0f;
	};
	struct FrameResource
	{
	public:
		NO_COPY(FrameResource);
		FrameResource(ID3D12Device* device, UINT passCount, UINT objectCount);
		~FrameResource()
		{
			Safe_Release(m_pObjectCB);
			Safe_Release(m_pPassCB);
			Safe_Release(CmdListAlloc);
		};

		// GPU가 명령을 다 처리한후 할당자를 재설정해야 하기 때문에 프레임마다 할당자 필요
		ComPtr<ID3D12CommandAllocator> CmdListAlloc;

		// 상수 버퍼는 GPU가 명령 다 처리한 후 갱신해야 해서 매 프레임 새로운 상수버퍼 필요
		
		CUploadBuffer<PassConstants>* m_pPassCB = nullptr;
		CUploadBuffer<ObjectConstants>* m_pObjectCB = nullptr;

		// Command 어디까지 해야 있는지 체크
		UINT64 Fence = 0;
	};
public:
	HRESULT Initialize();
	void Pipeline_Tick(_float fDeltaTime);
	void Render();
	HRESULT Free() override;
public:
	HRESULT Re_Initialize();
	// TODO : Build PSO Overrides
	HRESULT Build_PSO(const D3D12_GRAPHICS_PIPELINE_STATE_DESC& pipeline_desc, Pipeline::ENUM_PSO psoIndex);
	void Push_InputLayout(D3D12_INPUT_ELEMENT_DESC desc, ENUM_InputLayout eEnum)
	{
		m_vecInputLayout[eEnum].push_back(desc);
	}
	void Update_ObjPipelineLayer(CGameObject* pObject, Pipeline::ENUM_PSO ePsoEnum);
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
	HRESULT Init_ConstantBuffersView();
	HRESULT Init_RootSignature();
	void Update_ObjectCBs(_float fDeltaTime);
	void Update_MainPassCB(_float fDeltaTime);
	

private: // Graphic Device
	class CGraphic_Device* m_pGraphic_Device = nullptr; // Singleton Class
	ComPtr<ID3D12Device> m_pDevice = nullptr; // Real Device
	ComPtr<ID3D12GraphicsCommandList> m_pCommandList = nullptr;
private: // Frame Resource
	//
	vector<FrameResource*> m_vecFrameResource;
	FrameResource* m_pCurFrameResource = nullptr;
	int m_iCurFrameResourceIndex = 0;

	//unordered_map<string, ComPtr<PassConstant>>  
	//unordered_map<string, ComPtr<ObjectConstant>>  
private: // Camera Property
	_float4x4 m_pipelineMatrix[ENUM_PIPELINE_MAT_END];
	//_float4 m_CamPosition;
private: // ConstantBuffer
	_uint m_iPassCBVOffset = 0;
	_uint m_iCbvSrvUavDescriptorSize = 0;
	ComPtr<ID3D12DescriptorHeap> m_pCbvHeap = nullptr;
	PassConstants m_MainPassCB;
private: // Input layout
	vector<D3D12_INPUT_ELEMENT_DESC> m_vecInputLayout[InputLayout_END];
private: // Root Signature
	ComPtr<ID3D12RootSignature> m_RootSigArr[RootSig_END];
private: //Shader Class
	map<wstring, class CShader*> m_map_Shader;
private: // PSO
	ComPtr<ID3D12PipelineState> m_PSOArr[Pipeline::PSO_END];
	PipelineLayer m_vecPipelineLayerArr[Pipeline::PSO_END]; // 게임 오브젝트의 Pipeline_Tick을 대신해 돌려주는 함수
	//CD3DX12_PIPELINE_STATE_STREAM_SUBOBJECT<>
private: // Pointer
	class CComponentManager* m_pComponentManager = nullptr;
	class CGameObjectManager* m_pGameObjectManager = nullptr;
	class CCamera* m_pActiveCam = nullptr;
};

_NAMESPACE