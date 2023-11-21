#include "CubeMesh.h"
#include "Transform.h"
#include "Cube.h"

#ifdef _DEBUG
_uint CCube::m_iClonedNum = 0;
#endif _DEBUG

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
#ifdef _DEBUG
	++m_iClonedNum;
#endif // DEBUG

	return nullptr;
}

HRESULT CCube::Initialize_Prototype()
{
	// ±âº» ÄÄÆ÷³ÍÆ® ºÎÂø
	Add_Component(L"Transform", reinterpret_cast<CComponent**>(&m_pTransform));
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

void CCube::Pipeline_Tick()
{
}

void CCube::Render()
{
}

HRESULT CCube::Free()
{
	Safe_Release(m_pCubeMeshCom);
	Safe_Release(m_pTransform);

	return CGameObject::Free();
}
