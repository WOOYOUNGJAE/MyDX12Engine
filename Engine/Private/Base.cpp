#include "Base.h"
#include "GameInstance.h";
#include "PipelineManager.h"

_ulong CBase::AddRef()
{
	if (dynamic_cast<CPipelineManager*>(this))
	{
		int a = 1;
	}
	return ++m_dwRefCnt;
}

unsigned long CBase::Release()
{
	if (dynamic_cast<CPipelineManager*>(this))
	{
		int a = 1;
	}
	if (m_dwRefCnt == 0)
	{
		Free();

		delete this;

		return 0;
	}

	return m_dwRefCnt--;

}
