#include "Base.h"

unsigned long CBase::Release()
{
	if (m_dwRefCnt == 0)
	{
		Free();

		delete this;

		return 0;
	}

	return m_dwRefCnt--;

}