#pragma once
#include "Component.h"
#include "UploadBuffer.h"
NAMESPACE_(Engine)

struct FrameResource;
// �������� ���� ����, ���ӿ�����Ʈ�� ������Ʈ�� ������ ���� Ŭ�еǴ� ���� �ƴ� ������
// ������Ʈ ������ �ǵ� : �ڱ� �ڽ��� ���� Ư�� ���� �׷쿡 �ֱ� ����
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
	// n���� �迭
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

	// GPU�� ����� �� ó������ �Ҵ��ڸ� �缳���ؾ� �ϱ� ������ �����Ӹ��� �Ҵ��� �ʿ�
	ID3D12CommandAllocator* pCmdListAlloc;

	// ��� ���۴� GPU�� ��� �� ó���� �� �����ؾ� �ؼ� �� ������ ���ο� ������� �ʿ�
	CUploadBuffer<OBJ_CONSTANT_BUFFER>* pObjectCB = nullptr;
	//std::unique_ptr<UploadBuffer<PassConstants>> PassCB = nullptr;

	// Command ������ �ؾ� �ִ��� üũ
	UINT64 Fence = 0;
};
_NAMESPACE
