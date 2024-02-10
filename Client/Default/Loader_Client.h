#pragma once
#include "Base.h"
#include "Client_Defines.h"

NAMESPACE_(Engine)
class CComponentManager;
class CComponent;
class CTexture;
_NAMESPACE


NAMESPACE_(Client)

class CClientLoader : public CBase
{
private:
	CClientLoader() = default;
	~CClientLoader() override = default;

public:
	static CClientLoader* Create();
	void Load(ID3D12Device* pDevice);
	void Load_Textures();
	HRESULT Free() override;
private:
	ID3D12Device* m_pDevice;
	CComponentManager* m_pCompoManager;
};

_NAMESPACE