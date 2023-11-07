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
	_ulong Add_Ref() { return ++m_dwRefCnt; }
	// RefCount ���� ��Ű�� ���� �� ����
	_ulong Release();

	// �Ҹ��� ����, ���������� �ڽĿ��� �θ��� �Ҹ��ڸ� ȣ���ϴ� ��� ���
	virtual HRESULT Free() = 0;
private:
	_ulong m_dwRefCnt = 0;
};
_NAMESPACE