#pragma once
#include "Base.h"
#include "Engine_Struct.h"

NAMESPACE_(Engine)

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

private:
	class CGraphic_Device* m_pGraphic_Device = nullptr;
};

_NAMESPACE