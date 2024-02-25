#pragma once
#include "Base.h"
#include "Client_Defines.h"

NAMESPACE_(Engine)
class CLoadHelper;
_NAMESPACE


NAMESPACE_(Client)

class CClientLoader : public CBase
{
private:
	CClientLoader() = default;
	~CClientLoader() override = default;

public:
	static CClientLoader* Create();
	void Load();
	void Load_Textures();
	void Load_3DModels();
	HRESULT Free() override;

private:
	CLoadHelper* m_pHelper = nullptr;
	wstring m_strTextureRootPath = L"..\\..\\Resources\\Textures\\";
	std::string m_str3DModelRootPath = "..\\..\\Resources\\3DModels\\";
};

_NAMESPACE