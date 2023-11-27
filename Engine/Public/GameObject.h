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
	virtual HRESULT Initialize(void* pArg = nullptr) = 0;
	virtual CGameObject* Clone(void* pArg = nullptr) = 0;
	virtual void Tick(_float fDeltaTime) = 0;
	virtual void Late_Tick(_float fDeltaTime) = 0;
	virtual void Pipeline_Tick(){};
	virtual void Render(){};
	HRESULT Free() override;
public:
	virtual int& Get_NumFrameDirtyRef() = 0;
	virtual _matrix Get_WorldMatrix() = 0;
	virtual _float3 Get_Pos() = 0;
public:
	virtual _bool Com_Already_Owned(const wstring& strComTag);
	// 이미 있는지, 컴포넌트 생성, 맵에 넣기, 
	virtual HRESULT Add_Component(const wstring& strComTag, class CComponent** ppOutCom, void* pArg = nullptr);

protected:
	map<wstring, CComponent*> m_mapComponents;
	_uint m_eENUM_PSO = 0; // PSO_DEFAULT
};

_NAMESPACE