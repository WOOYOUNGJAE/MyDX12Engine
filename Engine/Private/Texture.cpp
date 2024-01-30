#include "Texture.h"

CTexture::CTexture()
{
}

CTexture::CTexture(const CTexture& rhs)
{
}

CTexture* CTexture::Create()
{
	return nullptr;
}

CComponent* CTexture::Clone(void* pArg)
{
	return nullptr;
}

HRESULT CTexture::Initialize_Prototype()
{
	return CComponent::Initialize_Prototype();
}

HRESULT CTexture::Initialize(void* pArg)
{
	return CComponent::Initialize(pArg);
}

HRESULT CTexture::Free()
{
	return CComponent::Free();
}
