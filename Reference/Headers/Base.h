#pragma once
#include "Engine_Defines.h"

NAMESPACE_(Engine)
class ENGINE_DLL CBase abstract
{
protected:
	CBase() = default;
	virtual ~CBase() = default;

public:
	// RefCount ���� ���� �� ����
	_ulong AddRef() { return ++m_dwRefCnt; }
	// RefCount ���� ��Ű�� ���� �� ����
	_ulong Release();

	// �Ҹ��� ����, ���������� �ڽĿ��� �θ��� �Ҹ��ڸ� ȣ���ϴ� ��� ���
	virtual HRESULT Free() = 0;

#ifdef _DEBUG
protected:
	static _uint m_iClonedNum;
//#ifdef _DEBUG Ȱ��
//	_uint Class::m_iClonedNum = 0;
//	++m_iClonedNum;
//#endif // DEBUG
#endif // _DEBUG

private:
	_ulong m_dwRefCnt = 0;
};
_NAMESPACE