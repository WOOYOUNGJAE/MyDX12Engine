#pragma once
#include "Base.h"

NAMESPACE_(Engine)
class CGameObject;
class CObjLayer;
using namespace std;

// Prototype�� �̹� �����ϸ� Crash, Layer�� �ڵ� �߰�
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
public:
	CGameObject* FindandGet_Prototype(const wstring& strTag);
	CGameObject* FindandGet_Cloned(const wstring& strProtoTag, UINT eLayerEnum = OBJ_LAYER_0, UINT iClonedNum = 1);
	CObjLayer* Find_Layer(UINT eLayerEnum);
	HRESULT Add_Prototype(const wstring& strTag, CGameObject* pInstance);
	// GameObj������ Clone����� ��� ���ִ� �Ű�ü ���� �Լ� (interface ����)
	CGameObject* Clone_GameObject(const wstring& strPrototypeTag, void* pArg = nullptr);
	// Client ���� G_Obj�� ������ ��, ���̾�� ����
	HRESULT Add_GameObject_InScene(const wstring& strPrototypeTag, UINT eLayerEnum, void* pArg = nullptr);
	HRESULT Add_GameObject_InScene(const wstring& strPrototypeTag, UINT eLayerEnum, CGameObject** pOutObj, void* pArg = nullptr);


private:
	map<wstring, CGameObject*> m_mapObjPrototypes;
	map<UINT, CObjLayer*> m_mapLayer; // L"LayerTag"
};

_NAMESPACE