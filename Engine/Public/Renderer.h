#pragma once
#include "Component.h"
#include "UploadBuffer.h"

NAMESPACE_(Engine)
struct FrameResource;
// 렌더링을 직접 수행, 게임오브젝트에 컴포넌트로 부착될 때는 클론되는 것이 아닌 참조만
// 컴포넌트 형식인 의도 : 자기 자신을 쉽게 특정 렌더 그룹에 넣기 위해
using namespace std;
class ENGINE_DLL CRenderer final : public CComponent
{
private:
	CRenderer() = default;
	~CRenderer() override = default;

public: // typedef
	enum RENDERGROUP { FIRST, NOLIGHT, NOBLEND, BLEND, UI, RENDERGROUP_END };
public:
	static CRenderer* Create();
	CComponent* Clone(void* pArg) override; // return this
	HRESULT Initialize_Prototype() override;
	HRESULT Initialize(void* pArg) override;
	HRESULT Build_FrameResource(); // Build FrameResource, Build CBV inside, SRV로드 후 실행
	void Update_PassCB();
	void Update_ObjCB(class CGameObject* pGameObj);
	void BeginRender();
	void MainRender();
	void EndRender();
	void Present();
	HRESULT Free() override;
public: // getter setter
	ID3D12GraphicsCommandList* Get_CmdList() { return m_pCommandList; }
	D3D12_GPU_DESCRIPTOR_HANDLE Get_CbvSrvUavStart_GPU();
	D3D12_CPU_DESCRIPTOR_HANDLE Get_CbvSrvUavStart_CPU();
	CD3DX12_GPU_DESCRIPTOR_HANDLE Get_HandleOffsettedGPU(INT iOffset);
	CD3DX12_GPU_DESCRIPTOR_HANDLE Get_ObjCbvHandleOffsettedGPU(UINT iOffsetPlus = 0);
	CD3DX12_GPU_DESCRIPTOR_HANDLE Get_PassCbvHandleOffsettedGPU();
	UINT Get_ObjCbvDescriptorSize() { return m_iObjCbvDescriptorSize; }
	void Set_ProjMat(const CAMERA_DESC& camDesc);
public: // Component Functions for GameObject
	void AddTo_RenderGroup(UINT IsFirst, UINT eCullMode, UINT eBlendModeEnum, UINT eShaderTypeEnum, UINT eRootsigTypeEnum, class CGameObject* pGameObject);
	void Flush_CommandQueue();
	
public: // FrameResource
	OBJECT_CB m_objectConstants{};
	vector<FrameResource*> m_vecFrameResource;
	FrameResource* m_pCurFrameResource = nullptr;
	UINT m_iCurFrameResourceIndex = 0;
private:
	// n차원 배열
	list<CGameObject*> m_RenderGroup[RENDER_PRIORITY_END][D3D12_CULL_MODE_END][RENDER_BLENDMODE_END][RENDER_SHADERTYPE_END][ROOTSIG_TYPE_END];
private: // pointer
	class CDeviceResource* m_pDeviceResource = nullptr;
	class CPipelineManager* m_pPipelineManager = nullptr;
private: // DX Resource
	ID3D12CommandAllocator* m_pCommandAllocator = nullptr;
	ID3D12GraphicsCommandList* m_pCommandList = nullptr;
	ID3D12CommandQueue* m_pCommandQueue = nullptr; // Pointer
	ID3D12DescriptorHeap* m_pRtvHeap = nullptr;
	ID3D12Resource** m_pRenderTargetArr = nullptr;
private: // Index and Offset
	UINT m_iFrameIndex = 0;
	UINT m_iObjCBVHeapStartOffset = 0;
	UINT m_iPassCBVHeapStartOffset = 0;
	UINT m_iObjCbvDescriptorSize = 0;
private: //Fence
	ID3D12Fence* m_pFence = nullptr;
	UINT64 m_iFenceValue = 0;
	HANDLE m_fenceEvent;
	QUEUE_FLUSH_DESC m_queue_flush_desc{};
private: // Pass Info
	class CCamera* m_pActiveCam = nullptr;
	Matrix m_mProj;
	FLOAT m_fAspectRatio = 0.f;
private:
};

struct FrameResource
{
	NO_COPY(FrameResource);
	FrameResource(ID3D12Device * pDevice, UINT iObjectCount, UINT iPassCount = 0);
	~FrameResource();

	// GPU가 명령을 다 처리한후 할당자를 재설정해야 하기 때문에 프레임마다 할당자 필요
	ID3D12CommandAllocator* pCmdListAlloc;

	// 상수 버퍼는 GPU가 명령 다 처리한 후 갱신해야 해서 매 프레임 새로운 상수버퍼 필요
	CUploadBuffer<OBJECT_CB>* pObjectCB = nullptr;
	CUploadBuffer<PASS_CB_VP>* pPassCB = nullptr;
	CUploadBuffer<PASS_CB_VP_LIGHT>* pPassCB_vp_light = nullptr;
	//std::unique_ptr<UploadBuffer<PassConstants>> PassCB = nullptr;

	// Command 어디까지 해야 있는지 체크
	UINT64 Fence = 0;
};
_NAMESPACE
