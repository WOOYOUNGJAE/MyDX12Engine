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

protected:
	CGameObjectManager() = default;
	~CGameObjectManager() override = default;

public:
	HRESULT Initialize();
	HRESULT Free() override;
	HRESULT Tick(_float fDeltaTime);
	HRESULT Late_Tick(_float fDeltaTime);
public:
	CGameObject* Find_Prototype(const wstring& strTag);
	CObjLayer* Find_Layer(const wstring& strTag);
	HRESULT Add_Prototype(const wstring& strTag, CGameObject* pInstance);
	// GameObj내부의 Clone기능을 대신 해주는 매개체 역할 함수 (interface 역할)
	CGameObject* Clone_GameObject(const wstring& strPrototypeTag, void* pArg = nullptr);
	// Client 에서 G_Obj를 생성할 때, 레이어에도 넣음
	HRESULT Add_GameObject_InScene(const wstring& strPrototypeTag, const wstring& strLayerTag, void* pArg = nullptr);


private:
	map<wstring, CGameObject*> m_mapObjPrototypes;
	map<wstring, CObjLayer*> m_mapLayer;
};

_NAMESPACE