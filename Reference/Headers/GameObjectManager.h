#pragma once
#include "Base.h"

NAMESPACE_(Engine)
class CGameObject;
class CObjLayer;
using namespace std;

// Prototype은 이미 존재하면 Crash, Layer은 자동 추가
class CGameObjectManager : public CBase
{
	DECLARE_SINGLETON(CGameObjectManager)

private:
	CGameObjectManager() = default;
	~CGameObjectManager() override = default;

public:
	HRESULT Initialize();
	HRESULT Free() override;
	HRESULT Tick(_float fDeltaTime);
	HRESULT Late_Tick(_float fDeltaTime);
	void Render_Tick();
public: // getter
	const map<wstring, CGameObject*>& Get_ObjPrototypeMap() { return m_mapObjPrototypes; }
	const vector<CGameObject*>& Get_vecTempClonedObj() { return m_vecTempClonedObj; }
public:
	CGameObject* FindandGet_Prototype(const wstring& strTag);
	CGameObject* FindandGet_Cloned(const wstring& strProtoTag, UINT eLayerEnum = OBJ_LAYER_0, UINT iClonedNum = 1);
	CObjLayer* Find_Layer(UINT eLayerEnum);
	HRESULT Add_Prototype(const wstring& strTag, CGameObject* pInstance);
	// GameObj내부의 Clone기능을 대신 해주는 매개체 역할 함수 (interface 역할)
	CGameObject* Clone_GameObject(const wstring& strPrototypeTag, void* pArg = nullptr);
	// Client 에서 G_Obj를 생성할 때, 레이어에도 넣음
	HRESULT Add_GameObject_InScene(const wstring& strPrototypeTag, UINT eLayerEnum, void* pArg = nullptr);
	HRESULT Add_GameObject_InScene(const wstring& strPrototypeTag, UINT eLayerEnum, CGameObject** pOutObj, void* pArg = nullptr);
	void Add_ClonedObj_To_Array_For_ShaderTable(CGameObject* pObjInstance);
	void Clear_ClonedObjArr(); // DXR ShaderTable 빌드에 필요 했던 ClonedList 삭제
private:
	map<wstring, CGameObject*> m_mapObjPrototypes;
	map<UINT, CObjLayer*> m_mapLayer; // L"LayerTag"
	vector<CGameObject*> m_vecTempClonedObj; // DXR ShaderTable에 필요한 Ordered 게임오브젝트 리스트, 테이블 빌드 후 클리어
};

_NAMESPACE