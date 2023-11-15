#pragma once
#include "Base.h"

NAMESPACE_(Engine)
class CGameObject;
class CObjLayer;
using namespace std;

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
	HRESULT Add_Prototype(const wstring& strTag, CGameObject* pInstance);
	CGameObject* Clone_GameObject(const wstring& strTag, void* pArg = nullptr);


private:
	map<wstring, CGameObject*> m_mapObjPrototypes;
	map<wstring, CObjLayer*> m_mapLayer;
};

_NAMESPACE