#include "PipelineManager.h"
#include "ComponentManager.h"
#include "Component.h"
#include "Transform.h"
#include "CubeMesh.h"
#include "Graphic_Device.h"
#include "Shader.h"
#include "Renderer.h"
#include "Texture.h"
#include "TriangleMesh.h"
#include "TriangleMesh_PT.h"
IMPLEMENT_SINGLETON(CComponentManager)

HRESULT CComponentManager::Initialize()
{
#pragma region Init_Basic_Components
	ID3D12Device* pDevice = CGraphic_Device::Get_Instance()->Get_Device().Get();
	CGraphic_Device::Get_Instance()->Reset_CmdList();
	Add_Prototype(L"CubeMesh", CCubeMesh::Create());
	Add_Prototype(L"TriangleMesh", CTriangleMesh::Create());
	Add_Prototype(L"TriangleMesh_PT", CTriangleMesh_PT::Create());
	// CShader
	{
		SHADER_INIT_DESC* pShader_desc = new SHADER_INIT_DESC[2]{};
		pShader_desc[0].filename = L"..\\Bin\\Shader\\color1.hlsl";
		pShader_desc[0].defines = nullptr;
		pShader_desc[0].entrypoint = "VS";
		pShader_desc[0].target = "vs_5_0";

		pShader_desc[1] = pShader_desc[0];
		pShader_desc[1].entrypoint = "PS";
		pShader_desc[1].target = "ps_5_0";
		Add_Prototype(L"Shader_Default", CShader::Create(pShader_desc, 2));

		pShader_desc[0].filename = L"..\\Bin\\Shader\\simpleShader.hlsl";
		pShader_desc[1].filename = L"..\\Bin\\Shader\\simpleShader.hlsl";
		Add_Prototype(L"Shader_Simple", CShader::Create(pShader_desc, 2));
		pShader_desc[0].filename = L"..\\Bin\\Shader\\simpleShader2.hlsl";
		pShader_desc[1].filename = L"..\\Bin\\Shader\\simpleShader2.hlsl";
		Add_Prototype(L"Shader_Simple2", CShader::Create(pShader_desc, 2));
		Safe_Delete_Array(pShader_desc);
	}

	Add_Prototype(L"Renderer", CRenderer::Create());


#pragma region Component with Descriptor
	Add_Prototype(L"Transform", CTransform::Create());
	//Texture
	/*{
		ResourceUploadBatch resourceUpload(pDevice);
		resourceUpload.Begin();

		TEXTURE_INIT_DESC texture_init_desc{};
		texture_init_desc.bIsCubeMap = false;
		texture_init_desc.pDevice = pDevice;
		texture_init_desc.pResourceUpload = &resourceUpload;
		texture_init_desc.strPath = L"..\\..\\Resources\\Textures\\checkboard.dds";

		Add_Prototype(L"Texture_Checkboard", CTexture::Create(&texture_init_desc));
		auto finish = resourceUpload.End(CGraphic_Device::Get_Instance()->Get_CommandQueue().Get());
		finish.wait();
	}*/
#pragma endregion


	CGraphic_Device::Get_Instance()->Close_CmdList();
	CGraphic_Device::Get_Instance()->Execute_CmdList();
#pragma endregion Init_Basic_Components
	

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
	Safe_AddRef(pInstance);

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
