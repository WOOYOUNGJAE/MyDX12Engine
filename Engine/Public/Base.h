#pragma once
#include "Engine_Defines.h"

NAMESPACE_(Engine)
class ENGINE_DLL CBase abstract
{
protected:
	CBase() = default;
	CBase(CBase& rhs) { m_iClonedNum = rhs.m_iClonedNum + 1; }
	virtual ~CBase() = default;

public:
	// RefCount ���� ���� �� ����
	_ulong AddRef();
	// RefCount ���� ��Ű�� ���� �� ����
	_ulong Release();

	// �Ҹ��� ����, ���������� �ڽĿ��� �θ��� �Ҹ��ڸ� ȣ���ϴ� ��� ���
	virtual HRESULT Free() = 0;

	UINT Get_ClonedNum() const { return m_iClonedNum; }
#ifdef _DEBUG
//#ifdef _DEBUG Ȱ��
//	_uint Class::m_iClonedNum = 0;
//	++m_iClonedNum;
//#endif // DEBUG
#endif // _DEBUG

protected:
	UINT m_iClonedNum = 0;
private:
	_ulong m_dwRefCnt = 0;
protected:
	_bool m_bIsCloned = false;
};
_NAMESPACE