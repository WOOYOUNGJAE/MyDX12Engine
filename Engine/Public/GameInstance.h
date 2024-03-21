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
	HRESULT Init_Engine(GRAPHIC_DESC& GraphicDesc, _Inout_ ID3D12Device** ppDevice);
	void Engine_Tick(FLOAT fDeltaTime);
	void Tick(_float fDeltaTime);
	void Late_Tick(_float fDeltaTime);
	void Render_Tick();
	static void Release_Engine();
public: // Manager Function Offer
	// GraphicDevice
	UINT Get_CbvSrvUavDescriptorSize() const;
	// ComponentManager
	HRESULT Add_ComPrototype(const wstring& strTag, class CComponent* pComInstance);
	CComponent* Clone_ComPrototype(const wstring& strTag, void* pArg = nullptr);
	// GameObjManager
	HRESULT Add_GameObjPrototype(const wstring& strTag, class CGameObject* pInstance);
	CGameObject* Clone_GameObject(const wstring& strPrototypeTag, void* pArg = nullptr);
	HRESULT Add_GameObject_InScene(const wstring& strPrototypeTag, UINT eLayerEnum, void* pArg = nullptr);
	HRESULT Add_GameObject_InScene(const wstring& strPrototypeTag, UINT eLayerEnum, CGameObject** pOutObj, void* pArg = nullptr);
	CGameObject* FindandGet_GameObj_Cloned(const wstring& strPrototypeTag, UINT eLayerEnum = OBJ_LAYER_0, UINT iClonedNum = 1);
	// Pipeline manager
	void Update_ObjPipelineLayer(CGameObject* pObject, _uint ePsoEnum);
	// CameraManger
	void Set_MainCam(wstring strName);
	// DXR
	HRESULT Init_DXR();
public:
	class CRenderer* Get_Renderer();

private:
	HWND* m_pHwndClient  = nullptr;
private: // Singleton
	class CDeviceResource* m_pDeviceResource = nullptr;
	class CComponentManager* m_pComponentManager = nullptr;
	class CGameObjectManager* m_pGameObjectManager = nullptr;
	class CPipelineManager* m_pPipelineManager = nullptr;
	class CAssetManager* m_pAssetManager = nullptr;
	class CD3DResourceManager* m_pD3DResourceManager = nullptr;
	class CInputManager* m_pInputManager = nullptr;
	class CCameraManager* m_pCameraManager = nullptr;
	class CLoadHelper* m_pLoadHelper = nullptr;
	class CDXRResource* m_pDxrResource = nullptr;
private:
	class CDXRRenderer* m_pDxrRenderer = nullptr;
};

_NAMESPACE