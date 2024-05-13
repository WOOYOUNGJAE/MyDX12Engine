#pragma once
#include "Base.h"

NAMESPACE_(Engine)

class CCamera;
// Ŭ�е� ī�޶� ����, map�� key�� ���̴� string�� ������Ÿ���� �ƴ� �Ŵ��������� �����Ǵ� Ű
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