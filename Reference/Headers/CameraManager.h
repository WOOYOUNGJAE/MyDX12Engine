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
	void Set_MainCam(wstring strName);
	CCamera* FindandGet(wstring strName);
	void Register(wstring strName, CCamera* pInstance);
private:
	std::map<wstring, CCamera*> m_mapCamera;
	CCamera* m_pMainCam = nullptr;

};

_NAMESPACE