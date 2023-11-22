#pragma once
#include "Engine_Defines.h"

NAMESPACE_(Engine)
class ENGINE_DLL CBase abstract
{
protected:
	CBase() = default;
	virtual ~CBase() = default;

public:
	// RefCount 증가 후의 값 리턴
	_ulong AddRef() { return ++m_dwRefCnt; }
	// RefCount 감소 시키기 이전 값 리턴
	_ulong Release();

	// 소멸자 역할, 예외적으로 자식에서 부모의 소멸자를 호출하는 경우 대비
	virtual HRESULT Free() = 0;

#ifdef _DEBUG
protected:
	static _uint m_iClonedNum;
//#ifdef _DEBUG 활용
//	_uint Class::m_iClonedNum = 0;
//	++m_iClonedNum;
//#endif // DEBUG
#endif // _DEBUG

private:
	_ulong m_dwRefCnt = 0;
};
_NAMESPACE