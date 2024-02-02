#pragma once
#include <unordered_map>

#include "Component.h"
NAMESPACE_(Engine)

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
	void BeginRender();
	void MainRender();
	void EndRender();
	void Present();
	HRESULT Free() override;
public:
	void AddTo_RenderGroup(RENDERGROUP eRenderGroup, class CGameObject* pGameObject);
	void AddTo_RenderGroup(UINT IsFirst, UINT eBlendModeEnum, UINT eShaderTypeEnum, UINT eParamComboEnum, class CGameObject* pGameObject);
private:
	// n���� �迭
	list<CGameObject*> m_RenderGroup[RENDER_PRIORITY_END][RENDER_BLENDMODE_END][RENDER_SHADERTYPE_END][RENDER_PARAMCOMBO_END];
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
	ID3D12RootSignature* m_pCurRootSig = nullptr;
	ID3D12PipelineState* m_pCurPSO = nullptr;
private:
	UINT m_iFrameIndex = 0;

private: //Fence
	ID3D12Fence* m_pFence = nullptr;
	UINT64 m_iFenceValue = 0;
	HANDLE m_fenceEvent;
	QUEUE_FLUSH_DESC m_queue_flush_desc{};
};
_NAMESPACE
