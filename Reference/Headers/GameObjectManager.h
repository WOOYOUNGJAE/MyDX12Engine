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
	// GameObj������ Clone����� ��� ���ִ� �Ű�ü ���� �Լ� (interface ����)
	CGameObject* Clone_GameObject(const wstring& strPrototypeTag, void* pArg = nullptr);
	// Client ���� G_Obj�� ������ ��, ���̾�� ����
	HRESULT Add_GameObject_InScene(const wstring& strPrototypeTag, const wstring& strLayerTag, void* pArg = nullptr);


private:
	map<wstring, CGameObject*> m_mapObjPrototypes;
	map<wstring, CObjLayer*> m_mapLayer;
};

_NAMESPACE