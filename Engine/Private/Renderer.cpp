#include "Renderer.h"

#include "GameObject.h"

CRenderer* CRenderer::Create()
{
	CRenderer* pInstance = new CRenderer();

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("CRenderer : Failed to Init Prototype");
	}

	return pInstance;
}

CComponent* CRenderer::Clone(void* pArg)
{
	CBase::AddRef();
	return this;
}

HRESULT CRenderer::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CRenderer::Initialize(void* pArg)
{
	return CComponent::Initialize(pArg);
}

void CRenderer::Render()
{
	for (auto& iter : m_RenderGroup)
	{
		for (auto& innerIter : iter)
		{
			innerIter->Render();
			Safe_Release(innerIter);
		}

		iter.clear(); // 그룹 내 렌더 끝나면 비우기
	}
}

HRESULT CRenderer::Free()
{	
	for (auto& iter : m_RenderGroup)
	{
		for (auto& innerIter : iter)
		{
			Safe_Release(innerIter);
		}
		iter.clear();
	}

	return CComponent::Free();
}

void CRenderer::AddTo_RenderGroup(RENDERGROUP eRenderGroup, CGameObject* pGameObject)
{
	if (eRenderGroup >= RENDERGROUP_END)
	{
		MSG_BOX("Invalid RenderGroup");
	}
	
	m_RenderGroup[eRenderGroup].push_back(pGameObject);

	Safe_AddRef(pGameObject);
}
