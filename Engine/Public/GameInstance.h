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
	HRESULT Init_Engine(const GRAPHIC_DESC& GraphicDesc, _Inout_ ID3D12Device** ppDevice);
	HRESULT Release_Engine();
public: // Manager Function Offer
	// ComponentManager
	HRESULT Add_ComPrototype(const wstring& strTag, class CComponent* pComInstance);
	CComponent* Clone_ComPrototye(const wstring& strTag, void* pArg = nullptr);

private:
	class CGraphic_Device* m_pGraphic_Device = nullptr;
	class CComponentManager* m_pComponentManager = nullptr;
};

_NAMESPACE