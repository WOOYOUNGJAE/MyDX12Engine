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
	// ±âº» ÄÄÆ÷³ÍÆ® ºÎÂø
	Add_Component(L"CubeMesh", reinterpret_cast<CComponent**>(&m_pCubeMeshCom));


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
	Safe_Release(m_pCubeMeshCom);

	return CGameObject::Free();
}
