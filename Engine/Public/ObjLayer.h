#pragma once
#include "Base.h"
// Layer ������Ʈ, ����Ʈ ����
NAMESPACE_(Engine)
class CObjLayer : public CBase
{
protected:
	CObjLayer() = default;
	~CObjLayer() override = default;

public:
	void Tick(_float fDeltaTime);
	HRESULT Late_Tick(_float fDeltaTime);
	HRESULT Free() override;
	
public:
	std::list<class CGameObject*> m_GameObjList;


};

_NAMESPACE