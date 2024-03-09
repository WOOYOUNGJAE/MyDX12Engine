#include "PipelineManager.h"
#include "ComponentManager.h"

#include "Asset.h"
#include "Component.h"
#include "Transform.h"
#include "CubeMesh.h"
#include "Graphic_Device.h"
#include "Material.h"
#include "Shader.h"
#include "Renderer.h"
#include "TextureCompo.h"
#include "TriangleMesh.h"
#include "TriangleMesh_PT.h"
#include "MeshObject.h"
#include "AssetManager.h"
#include "CylinderMesh.h"
#include "GridMesh.h"
#include "SingleVertexMesh.h"
#include "SphereMesh.h"
IMPLEMENT_SINGLETON(CComponentManager)

HRESULT CComponentManager::Initialize()
{
#pragma region Init_Basic_Components
	ID3D12Device* pDevice = CGraphic_Device::Get_Instance()->Get_Device();
	CGraphic_Device::Get_Instance()->Reset_CmdList();

	Add_Prototype(L"Transform", CTransform::Create());

	Add_Prototype(L"MeshObject", CMeshObject::Create());
	// CShader
	{
		SHADER_INIT_DESC shaderInitDesc[CShader::SHADER_TYPE_END]{};
		shaderInitDesc[0].filename = L"..\\Bin\\Shader\\simpleShader.hlsl";
		shaderInitDesc[0].defines = nullptr;
		shaderInitDesc[0].inputLayout = SHADER_INIT_DESC::POS_COLOR;
		shaderInitDesc[1] = shaderInitDesc[0];
		shaderInitDesc[0].entrypoint = "VS";
		shaderInitDesc[1].entrypoint = "PS";
		shaderInitDesc[2].entrypoint = "GS";
		shaderInitDesc[0].target = "vs_5_0";
		shaderInitDesc[1].target = "ps_5_0";
		shaderInitDesc[2].target = "gs_5_0";
		Add_Prototype(L"Shader_Simple", CShader::Create(shaderInitDesc, 2));

		shaderInitDesc[0].filename = L"..\\Bin\\Shader\\simpleShader2.hlsl";
		shaderInitDesc[1].filename = shaderInitDesc[0].filename;
		shaderInitDesc[0].inputLayout = SHADER_INIT_DESC::POS_NORMAL_TEXCOORD;
		Add_Prototype(L"Shader_Simple2", CShader::Create(shaderInitDesc, 2));

		// SimpleShader3
		shaderInitDesc[0].filename = L"..\\Bin\\Shader\\simpleShader3.hlsl";
		shaderInitDesc[1].filename = shaderInitDesc[0].filename;
		shaderInitDesc[0].inputLayout = SHADER_INIT_DESC::POS_NORMAL_TEXCOORD;
		Add_Prototype(L"Shader_Simple3", CShader::Create(shaderInitDesc, 2));

		// skybox.hlsl
		shaderInitDesc[0].filename = L"..\\Bin\\Shader\\skybox.hlsl";
		shaderInitDesc[1].filename = shaderInitDesc[0].filename;
		shaderInitDesc[0].inputLayout = SHADER_INIT_DESC::POS_NORMAL_TEXCOORD;
		Add_Prototype(L"Shader_skybox", CShader::Create(shaderInitDesc, 2));

		// treeBillboard.hlsl
		shaderInitDesc[0].filename = L"..\\Bin\\Shader\\treeBillboard.hlsl";
		shaderInitDesc[1].filename = shaderInitDesc[0].filename;
		shaderInitDesc[2].filename = shaderInitDesc[0].filename;
		shaderInitDesc[0].inputLayout = SHADER_INIT_DESC::POS_SIZE;
		Add_Prototype(L"Shader_treeBillboard", CShader::Create(shaderInitDesc, 3));

	}

	Add_Prototype(L"Material", CMaterial::Create());

	
	//Texture
	Add_Prototype(L"Renderer", CRenderer::Create());
	Add_Prototype(L"Texture", CTextureCompo::Create());

	
#pragma endregion Init_Basic_Components
	
	CAssetManager* pAssetManager = CAssetManager::Get_Instance();

#pragma region MeshData
	pAssetManager->Add_MeshDataPrototype(L"CubeMesh", CCubeMesh::Create());
	pAssetManager->Add_MeshDataPrototype(L"TriangleMesh", CTriangleMesh::Create());
	pAssetManager->Add_MeshDataPrototype(L"TriangleMesh_PT", CTriangleMesh_PT::Create());
	pAssetManager->Add_MeshDataPrototype(L"Grid_1X1", CGridMesh::Create(1, 1));
	pAssetManager->Add_MeshDataPrototype(L"Grid_10X10", CGridMesh::Create(10, 10));
	pAssetManager->Add_MeshDataPrototype(L"Cylinder_20_05_05", CCylinderMesh::Create(20, 0.5f, 0.5f));
	pAssetManager->Add_MeshDataPrototype(L"Sphere_15X15", CSphereMesh::Create(15, 15));
	pAssetManager->Add_MeshDataPrototype(L"SingleVertexMesh", CSingleVertexMesh::Create());
#pragma endregion



	CGraphic_Device::Get_Instance()->Close_CmdList();
	CGraphic_Device::Get_Instance()->Execute_CmdList();

	return S_OK;
}

CComponent* CComponentManager::FindandGet_Prototype(const wstring& strTag)
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
	if (FindandGet_Prototype(strTag))
	{
		MSG_BOX("ComponentManager: Protype Already Exists");
		return E_FAIL;
	}
	m_mapComPrototypes.emplace(strTag, pComInstance);

	return S_OK;
}

CComponent* CComponentManager::Clone_Component(const wstring& strTag, void* pArg)
{
	CComponent* pInstance = FindandGet_Prototype(strTag);

	if (pInstance == nullptr)
	{	
		MSG_BOX("ComponentManager: Cannot Find Prototype");
	}

	return pInstance->Clone(pArg);
}

CRenderer* CComponentManager::Get_Renderer()
{
	CRenderer* pInstance = static_cast<CRenderer*>(FindandGet_Prototype(L"Renderer"));

	return pInstance;
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
