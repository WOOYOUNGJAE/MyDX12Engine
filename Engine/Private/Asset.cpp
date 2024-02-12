#include "Asset.h"

HRESULT CAsset::Free()
{
	Safe_Release(m_pAssetData);

	return S_OK;
}
