#include "ComponentManager.h"
#include "Component.h"
#include "Transform.h"
#include "CubeMesh.h"
#include "Shader.h"
IMPLEMENT_SINGLETON(CComponentManager)

HRESULT CComponentManager::Initialize()
{
#pragma region Init_Basic_Components
	Add_Prototype(L"Transform", CTransform::Create());
	Add_Prototype(L"CubeMesh", CCubeMesh::Create());
	// CShader
	SHADER_INIT_DESC shader_desc{};
	shader_desc.filename = L"../Bin/Shader/vShader_Default.hlsl";
	shader_desc.defines = nullptr;
	shader_desc.entrypoint = "main";
	shader_desc.target = "vs_5_0";
	Add_Prototype(L"vShader_Default", CShader::Create(shader_desc));

#pragma endregion Init_Basic_Components


	return S_OK;
}

CComponent* CComponentManager::Find_Prototype(const wstring& strTag)
{
	auto iter = m_mapComPrototypes.find(strTag);

	// 존재하지 않는다면
	if (iter == m_mapComPrototypes.end())
	{
		return nullptr;
	}

	return iter->second;
}

HRESULT CComponentManager::Add_Prototype(const wstring& strTag, CComponent* pComInstance)
{
	// 이미 존재한다면
	if (Find_Prototype(strTag))
	{
		MSG_BOX("ComponentManager: Protype Already Exists");
		return E_FAIL;
	}

	m_mapComPrototypes.emplace(strTag, pComInstance);

	return S_OK;
}

CComponent* CComponentManager::Clone_Component(const wstring& strTag, void* pArg)
{
	CComponent* pInstance = Find_Prototype(strTag);

	if (pInstance == nullptr)
	{	
		MSG_BOX("ComponentManager: Cannot Find Prototype");
	}

	return pInstance->Clone(pArg);
}

HRESULT CComponentManager::Free()
{
	// Free Map

	for (auto& iter : m_mapComPrototypes)
	{
		Safe_Release(iter.second);
	}

	m_mapComPrototypes.clear();

	return S_OK;
}
