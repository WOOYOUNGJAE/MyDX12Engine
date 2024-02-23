#include "Material.h"

CMaterial* CMaterial::Create()
{
	return new CMaterial;
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

	m_materialInfo = *reinterpret_cast<MATERIAL_INFO*>(pArg);

	return hr;
}

HRESULT CMaterial::Free()
{
	return CComponent::Free();
}
