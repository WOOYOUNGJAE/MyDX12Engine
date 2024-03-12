#include "Component.h"
#include "DeviceResource.h"


CComponent::CComponent()
{
}

HRESULT CComponent::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CComponent::Initialize(void* pArg)
{
	return S_OK;
}

HRESULT CComponent::Free()
{
	return S_OK;
}
