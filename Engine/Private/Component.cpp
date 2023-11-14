#include "Component.h"
#include "Graphic_Device.h"

CComponent::CComponent() : m_pDevice(CGraphic_Device::Get_Instance()->Get_Device())
{
	//ID3D12Device가 Comptr라 AddRef 안함
}

HRESULT CComponent::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CComponent::Initialize()
{
	return S_OK;
}

HRESULT CComponent::Free()
{
	return S_OK;
}
