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
	// RefCount 증가 후의 값 리턴
	_ulong AddRef();
	// RefCount 감소 시키기 이전 값 리턴
	_ulong Release();

	// 소멸자 역할, 예외적으로 자식에서 부모의 소멸자를 호출하는 경우 대비
	virtual HRESULT Free() = 0;

	UINT Get_ClonedNum() const { return m_iClonedNum; }
#ifdef _DEBUG
//#ifdef _DEBUG 활용
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