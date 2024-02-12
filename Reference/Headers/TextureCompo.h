#pragma once
#include "Component.h"

NAMESPACE_(Engine)

class ENGINE_DLL CTextureCompo : public CComponent
{
private:
	CTextureCompo();
	//CTextureCompo(const CTextureCompo& rhs);
	~CTextureCompo() override = default;

public:
	static CTextureCompo* Create();
	CComponent* Clone(void* pArg) override;
	HRESULT Initialize(void* pArg) override;
	HRESULT Free() override;

public: // for Prototype
	UINT64 m_iCbvSrvUavHeapOffset = 0;
private:
	wstring m_strTextureName;
};

_NAMESPACE
