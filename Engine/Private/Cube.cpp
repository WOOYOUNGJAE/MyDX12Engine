#include "CubeMesh.h"
#include "Cube.h"

CCube* CCube::Create()
{
	CCube* pInstance = new CCube;

	if (pInstance)
	{
		pInstance->Initialize_Prototype();
	}

	return pInstance;
}

CGameObject* CCube::Clone(void* pArg)
{
	
	return nullptr;
}

HRESULT CCube::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CCube::Initialize(void* pArg)
{
	return S_OK;
}

void CCube::Tick(_float fDeltaTime)
{
}

void CCube::Late_Tick(_float fDeltaTime)
{
}

void CCube::Render()
{
}

HRESULT CCube::Free()
{
	return CGameObject::Free();
}
