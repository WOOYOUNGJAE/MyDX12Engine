#pragma once
#include "Component.h"

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
	HRESULT Build_FrameResource(UINT inNumAllRenderingObject = 30); // Build FrameResource, Build CBV inside, SRV�ε� �� ����
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
	
private: // FrameResource
	vector<FrameResource*> m_vecFrameResource;
	FrameResource* m_pCurFrameResource = nullptr;
	UINT m_iCurFrameResourceIndex = 0;
private:
	// n���� �迭
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
	Matrix m_mProj;
	FLOAT m_fAspectRatio = 0.f;
private:
};

_NAMESPACE
