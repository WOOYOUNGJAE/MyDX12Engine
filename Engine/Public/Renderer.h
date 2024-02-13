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
	void Update_ObjCB(class CGameObject* pGameObj);
	void BeginRender();
	void MainRender();
	void EndRender();
	void Present();
	HRESULT Free() override;
public: // Component Functions for GameObject
	void AddTo_RenderGroup(UINT IsFirst, UINT eBlendModeEnum, UINT eShaderTypeEnum, UINT eRootsigTypeEnum, class CGameObject* pGameObject);
	//void Bind_Resource();
	
public: // FrameResource
	OBJ_CONSTANT_BUFFER m_objectConstants{};
	ID3D12Resource* m_pObjConstantsResource = nullptr;
	vector<FrameResource*> m_vecFrameResource;
	FrameResource* m_pCurFrameResource = nullptr;
	UINT m_iCurFrameResourceIndex = 0;
private:
	// n차원 배열
	list<CGameObject*> m_RenderGroup[RENDER_PRIORITY_END][RENDER_BLENDMODE_END][RENDER_SHADERTYPE_END][ROOTSIG_TYPE_END];
private: // pointer
	class CGraphic_Device* m_pGraphic_Device = nullptr;
	class CPipelineManager* m_pPipelineManager = nullptr;
private: // DX Resource
	ID3D12CommandAllocator* m_pCommandAllocator = nullptr;
	ID3D12GraphicsCommandList* m_pCommandList = nullptr;
	ID3D12CommandQueue* m_pCommandQueue = nullptr;
	ID3D12DescriptorHeap* m_pRtvHeap = nullptr;
	ID3D12Resource** m_pRenderTargetArr = nullptr;
private:
	UINT m_iFrameIndex = 0;
	UINT m_iCBVHeapStartOffset = 0;
private: //Fence
	ID3D12Fence* m_pFence = nullptr;
	UINT64 m_iFenceValue = 0;
	HANDLE m_fenceEvent;
	QUEUE_FLUSH_DESC m_queue_flush_desc{};
};

struct FrameResource
{
	NO_COPY(FrameResource);
	FrameResource(ID3D12Device * pDevice, UINT iObjectCount, UINT iPassCount = 0);
	~FrameResource();

	// GPU가 명령을 다 처리한후 할당자를 재설정해야 하기 때문에 프레임마다 할당자 필요
	ID3D12CommandAllocator* pCmdListAlloc;

	// 상수 버퍼는 GPU가 명령 다 처리한 후 갱신해야 해서 매 프레임 새로운 상수버퍼 필요
	CUploadBuffer<OBJ_CONSTANT_BUFFER>* pObjectCB = nullptr;
	//std::unique_ptr<UploadBuffer<PassConstants>> PassCB = nullptr;

	// Command 어디까지 해야 있는지 체크
	UINT64 Fence = 0;
};
_NAMESPACE
