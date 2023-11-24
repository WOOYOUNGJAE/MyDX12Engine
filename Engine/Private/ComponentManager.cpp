#include "ComponentManager.h"
#include "Component.h"
#include "Transform.h"
#include "CubeMesh.h"
#include "Shader.h"
#include "Renderer.h"
#include "Pipeline.h"
IMPLEMENT_SINGLETON(CComponentManager)

HRESULT CComponentManager::Initialize()
{
#pragma region Init_Basic_Components
	Add_Prototype(L"Transform", CTransform::Create());
	Add_Prototype(L"CubeMesh", CCubeMesh::Create());
	// CShader
	{
		SHADER_INIT_DESC* pShader_desc = new SHADER_INIT_DESC[2]{};
		pShader_desc[0].filename = L"..\\Bin\\Shader\\color0.hlsl";
		pShader_desc[0].defines = nullptr;
		pShader_desc[0].entrypoint = "VS";
		pShader_desc[0].target = "vs_5_0";

		pShader_desc[1] = pShader_desc[0];
		pShader_desc[1].entrypoint = "PS";
		pShader_desc[1].target = "ps_5_0";
		Add_Prototype(L"Shader_Default", CShader::Create(pShader_desc, 2));
		Safe_Delete_Array(pShader_desc);		
		//Add_Prototype(L"vShader_Default", CShader::Create(shader_desc));
	}

	Add_Prototype(L"Renderer", CRenderer::Create());
	
#pragma endregion Init_Basic_Components

	D3D12_INPUT_ELEMENT_DESC input_layout_desc[2]
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
	CPipeline* pPipeline = CPipeline::Get_Instance();
	for (auto& iter : input_layout_desc)
	{
		pPipeline->Push_InputLayout(iter, CPipeline::InputLayout_DEFAULT);
	}

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
