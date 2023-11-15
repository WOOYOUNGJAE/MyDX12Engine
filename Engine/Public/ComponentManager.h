#pragma once
#include "Base.h"

NAMESPACE_(Engine)
using namespace std;
//typedef map<wstring, class CComponent*> COM_PROTOTYPE;
class CComponentManager : public CBase
{
	DECLARE_SINGLETON(CComponentManager)
protected:
	CComponentManager() = default;
	~CComponentManager() override = default;

public:
	HRESULT Initialize();
	HRESULT Free() override;
public:
	class CComponent* Find_Prototype(const wstring& strTag);
	HRESULT Add_Prototype(const wstring& strTag, CComponent* pComInstance);
	CComponent* Clone_Component(const wstring& strTag, void* pArg = nullptr);
	

private:
	map<wstring, CComponent*> m_mapComPrototypes;
};

_NAMESPACE