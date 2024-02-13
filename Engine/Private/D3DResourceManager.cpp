#include "D3DResourceManager.h"

IMPLEMENT_SINGLETON(CD3DResourceManager)

void CD3DResourceManager::Register_Resource(UINT eType, ID3D12Resource** ppResource)
{
	m_vecID3D12ResourceArr[eType].push_back(*ppResource);
	Safe_AddRef(*ppResource);
	++m_iCurRegisterIndexArr[eType];
}

HRESULT CD3DResourceManager::Free()
{
	for (auto& arrIter : m_vecID3D12ResourceArr)
	{
		for (auto& vecIter : arrIter)
		{
			Safe_Release(vecIter);
		}
		arrIter.clear();
	}
	return S_OK;
}

ID3D12Resource* CD3DResourceManager::Get_Resource(UINT eType, UINT iRegisterIndex)
{
	return m_vecID3D12ResourceArr[eType][iRegisterIndex];
}
