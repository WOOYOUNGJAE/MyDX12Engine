#pragma once
#include "Base.h"
#include "Engine_Struct.h"

NAMESPACE_(Engine)
using namespace std;
class ENGINE_DLL CGameInstance : public CBase
{
	DECLARE_SINGLETON(CGameInstance)

private:
	CGameInstance();
	virtual ~CGameInstance() override = default;

public:
	HRESULT Free() override;

public:
	HRESULT Init_Engine(const GRAPHIC_DESC& GraphicDesc, _Inout_ ID3D12Device** ppDevice);
	void Engine_Tick(FLOAT fDeltaTime);
	void Tick(_float fDeltaTime);
	void Late_Tick(_float fDeltaTime);
	void Render_Tick();
	static void Release_Engine();
public: // Manager Function Offer
	// ComponentManager
	HRESULT Add_ComPrototype(const wstring& strTag, class CComponent* pComInstance);
	CComponent* Clone_ComPrototype(const wstring& strTag, void* pArg = nullptr);
	// GameObjManager
	HRESULT Add_GameObjPrototype(const wstring& strTag, class CGameObject* pInstance);
	CGameObject* Clone_GameObject(const wstring& strPrototypeTag, void* pArg = nullptr);
	HRESULT Add_GameObject_InScene(const wstring& strPrototypeTag, const wstring& strLayerTag, void* pArg = nullptr);
	HRESULT Add_GameObject_InScene(const wstring& strPrototypeTag, const wstring& strLayerTag, CGameObject** pOutObj , void* pArg = nullptr);
	// Pipeline manager
	void Update_ObjPipelineLayer(CGameObject* pObject, _uint ePsoEnum);
public:
	class CRenderer* Get_Renderer();

private:
	class CGraphic_Device* m_pGraphic_Device = nullptr;
	class CComponentManager* m_pComponentManager = nullptr;
	class CGameObjectManager* m_pGameObjectManager = nullptr;
	class CPipelineManager* m_pPipelineManager = nullptr;
	class CLoadHelper* m_pLoadHelper = nullptr;
};

_NAMESPACE