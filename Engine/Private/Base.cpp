#include "Base.h"
#include "GameInstance.h";
#include "PipelineManager.h"

_ulong CBase::AddRef()
{
	return ++m_dwRefCnt;
}

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
