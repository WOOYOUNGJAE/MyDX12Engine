#pragma once
#include "Base.h"

NAMESPACE_(Engine)

class CCamera;
// 클론된 카메라 관리, map의 key로 쓰이는 string은 프로토타입이 아닌 매니저에서만 관리되는 키
class ENGINE_DLL CCameraManager : public CBase
{
	DECLARE_SINGLETON(CCameraManager)
protected:
	CCameraManager() = default;
	~CCameraManager() override = default;
public:
	HRESULT Free() override;
	CCamera* Get_MainCam() { return m_pMainCam; }
#if DXR_ON
	void Set_MainCam(wstring strName, class CDXRRenderer* pDXRRenderer);
#else
	void Set_MainCam(wstring strName);
#endif
	CCamera* FindandGet(wstring strName);
	void Register(wstring strName, CCamera* pInstance);
private:
	std::map<wstring, CCamera*> m_mapCamera;
	CCamera* m_pMainCam = nullptr;

};

_NAMESPACE