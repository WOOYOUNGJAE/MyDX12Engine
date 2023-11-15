#pragma once
#include "Base.h"

NAMESPACE_(Engine)
using namespace std;
class ENGINE_DLL CGameObject abstract : public CBase
{
protected:
	CGameObject() = default;
	~CGameObject() override = default;

public: // LifeCycle
	virtual HRESULT Initialize_Prototype() = 0;
	virtual HRESULT Initialize() = 0;
	virtual HRESULT Tick(_float fDeltaTime) = 0;
	virtual HRESULT Late_Tick(_float fDeltaTime) = 0;
	virtual HRESULT Render() = 0;
	HRESULT Free() override;
public:
	virtual _bool Com_Already_Owned(const wstring& strComTag);
	// 이미 있는지, 컴포넌트 생성, 맵에 넣기, 
	virtual HRESULT Add_Component(const wstring& strComTag, class CComponent** ppOutCom, void* pArg = nullptr);

protected:
	//ID3D12Device*
	map<wstring, class CComponent*> m_mapComponents;
};

_NAMESPACE