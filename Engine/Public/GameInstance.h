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
	HRESULT Scene_Start(UINT iNumAllRenderingObject); // 씬 게임오브젝트 모두 생성 후 파악된 게임오브젝트 개수 바탕 F-Resource 빌드
	// GameObjManager
	HRESULT Add_GameObjPrototype(const wstring& strTag, class CGameObject* pInstance);
	CGameObject* Clone_GameObject(const wstring& strPrototypeTag, void* pArg = nullptr);
	HRESULT Add_GameObject_InScene(const wstring& strPrototypeTag, UINT eLayerEnum, void* pArg = nullptr);
	HRESULT Add_GameObject_InScene(const wstring& strPrototypeTag, UINT eLayerEnum, CGameObject** pOutObj, void* pArg = nullptr);
	CGameObject* FindandGet_GameObj_Cloned(const wstring& strPrototypeTag, UINT eLayerEnum = OBJ_LAYER_0, UINT iClonedNum = 1);
	void Add_ClonedObj_To_Array_For_ShaderTable(CGameObject* pObjInstance);
	void Clear_ClonedObjArray(); // DXR ShaderTable 빌드에 필요 했던 ClonedList 삭제
	// Pipeline manager
	void Update_ObjPipelineLayer(CGameObject* pObject, _uint ePsoEnum);
	// CameraManger
	void Set_MainCam(wstring strName);
	// Spatial Data Structure Manager
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
	class CSDSManager* m_pSDSManager = nullptr;
	class CLoadHelper* m_pLoadHelper = nullptr;
	class CFrameResourceManager* m_pFrameResourceManager = nullptr;
	//class 
#if DXR_ON
public:
	HRESULT Init_DXR();
	// GameObject 배열은 렌더링 오브젝트만 들고 옴
	void Build_AccelerationStructureTree(CGameObject** pGameObjArr, UINT iArrSize);
	class CDXRRenderer* Get_DXRRenderer();
private:
	class CDXRResource* m_pDxrResource = nullptr;
	class CDXRRenderer* m_pDxrRenderer = nullptr;
#endif
};

_NAMESPACE