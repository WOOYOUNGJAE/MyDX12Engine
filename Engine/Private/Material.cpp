#include "Material.h"

CMaterial* CMaterial::Create()
{
	CMaterial* pInstance = new CMaterial;
	pInstance->m_bIsPrototype = true;

	return pInstance;
}

CComponent* CMaterial::Clone(void* pArg)
{
	CMaterial* pInstance = new CMaterial(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		Safe_Release(pInstance);
		return nullptr;
	}

	return pInstance;
}

HRESULT CMaterial::Initialize(void* pArg)
{
	HRESULT hr = CComponent::Initialize(pArg);

#if DXR_ON
	struct MATERIAL_INFO_2ELEMENTS
	{
		MATERIAL_INFO pRastMatInfo;
		DXR::MATERIAL_INFO pDXRMatInfo;
	};
	m_materialInfo = (reinterpret_cast<MATERIAL_INFO_2ELEMENTS*>(pArg))->pRastMatInfo;
	m_DXR_materialInfo = (reinterpret_cast<MATERIAL_INFO_2ELEMENTS*>(pArg))->pDXRMatInfo;
#else
	m_materialInfo = *reinterpret_cast<MATERIAL_INFO*>(pArg);
#endif

	return hr;
}

HRESULT CMaterial::Free()
{
	return CComponent::Free();
}
