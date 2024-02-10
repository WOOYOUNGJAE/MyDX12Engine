#pragma once
#include <unordered_map>

#include "Base.h"
#include "UploadBuffer.h"

NAMESPACE_(Engine)
using namespace std;
using namespace Pipeline;
// Manage RootSig, PSO
// CGraphic_Device's Friend
class CPipelineManager : public CBase
{
	friend class CRenderer;
	DECLARE_SINGLETON(CPipelineManager)
private:
	CPipelineManager();
	~CPipelineManager() override = default;

public: // typedef
	//typedef ComPtr<ID3D12PipelineState> PSO;
	typedef list<class CGameObject*> PipelineLayer;
	enum ENUM_PIPELINE_MAT {VIEW_MAT, PROJ_MAT, ENUM_PIPELINE_MAT_END};
	enum ENUM_InputLayout {InputLayout_DEFAULT, InputLayout_END	};

public:
	HRESULT Initialize();
	HRESULT Free() override;
public: // RootSig and PSO
	BOOL PSO_Exist(const wstring& strKey) { return m_mapPSO.find(strKey) != m_mapPSO.end(); }
	void Add_NewPSO(const wstring& strKey, ID3D12PipelineState* pPSO) { m_mapPSO.emplace(strKey, pPSO); }

	// getter
	ID3D12RootSignature* Get_RootSig(const wstring& strKey);
	ID3D12RootSignature* Get_RootSig(UINT eRootSigType);
	ID3D12PipelineState* Get_PSO(const wstring& strKey);
	ID3D12PipelineState* Get_PSO(UINT IsFirst, UINT eBlendModeEnum, UINT eShaderTypeEnum, UINT eParamTypeEnum);

	void Update_ObjPipelineLayer(CGameObject* pObject, Pipeline::ENUM_PSO ePsoEnum);
	

private: // Graphic Device
	class CGraphic_Device* m_pGraphic_Device = nullptr; // Singleton Class
	ComPtr<ID3D12Device> m_pDevice = nullptr; // Real Device
private: // Root Signature
	
	vector<D3D12_INPUT_ELEMENT_DESC> m_vecInputLayoutArr[RENDER_SHADERTYPE_END];
	
	ID3D12RootSignature* m_rootSigArr[ROOTSIG_TYPE_END];
private: // PSO
	ID3D12PipelineState* m_PSOsArr[RENDER_PRIORITY_END][RENDER_BLENDMODE_END][RENDER_SHADERTYPE_END][ROOTSIG_TYPE_END];
	map<wstring, ID3D12PipelineState*> m_mapPSO;
	PipelineLayer m_vecPipelineLayerArr[Pipeline::PSO_END]; // 게임 오브젝트의 Pipeline_Tick을 대신해 돌려주는 함수
private: // Pointer
	class CComponentManager* m_pComponentManager = nullptr;
	class CGameObjectManager* m_pGameObjectManager = nullptr;
	class CCamera* m_pActiveCam = nullptr;
};

_NAMESPACE