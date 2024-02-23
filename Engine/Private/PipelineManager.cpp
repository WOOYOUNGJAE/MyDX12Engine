#include "PipelineManager.h"
#include "Renderer.h"
#include "ComponentManager.h"
#include "GameObjectManager.h"
#include "GameObject.h"
#include "Shader.h"
#include "Graphic_Device.h"
#include "Transform.h"
#include "Camera.h"
IMPLEMENT_SINGLETON(CPipelineManager)

_uint g_iNumFrameResource = 1; // extern frame resource num;
CPipelineManager::CPipelineManager() :
m_pComponentManager(CComponentManager::Get_Instance()),
m_pGameObjectManager(CGameObjectManager::Get_Instance())
{
	ZeroMemory(m_PSOsArr, sizeof(m_PSOsArr));
	ZeroMemory(m_rootSigArr, sizeof(m_rootSigArr));
	/*Safe_AddRef(m_pComponentManager);
	Safe_AddRef(m_pGameObjectManager);*/
}

HRESULT CPipelineManager::Initialize()
{
	// GraphicDevice들의 멤버들이 초기화 된 후 참조
	m_pGraphic_Device = CGraphic_Device::Get_Instance();
	m_pDevice = m_pGraphic_Device->Get_Device();
	Safe_AddRef(m_pGraphic_Device);

	HRESULT hr = S_OK;
#pragma region Build Root Signature

#pragma region _RootSig_Simple
	//CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc;
	//rootSigDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	//// 상수 버퍼 하나로 구성된 서술자 구간을 가리키는, 슬롯하나로 이루어진 루트 시그니쳐 생성
	//ComPtr<ID3DBlob> serializedRootSig;
	//ComPtr<ID3DBlob> errorBlob;

	//hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
	//	serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());
	//if (FAILED(hr)) { return hr; }

	//if (errorBlob)
	//{
	//	::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	//}

	//hr = m_pDevice->CreateRootSignature(
	//	0,
	//	serializedRootSig->GetBufferPointer(),
	//	serializedRootSig->GetBufferSize(),
	//	IID_PPV_ARGS(&m_rootSigArr[ROOTSIG_DEFAULT]));
	//if (FAILED(hr)) { return hr; }
#pragma endregion

#pragma region _Rootsig_TextureSampler
	{
		// [][]
		D3D12_FEATURE_DATA_ROOT_SIGNATURE RSFeatureData = {};

		// This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
		RSFeatureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

		hr = m_pDevice->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &RSFeatureData, sizeof(RSFeatureData));
		if (FAILED(hr)) // 1_1 버전 지원하는지
		{
			RSFeatureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
		}

		//CD3DX12_DESCRIPTOR_RANGE1 ranges[1]; // DescriptorTable, RootDescriptor, RootConstant 가 될 수 있음(InitAs..)
		//ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

		//CD3DX12_ROOT_PARAMETER1 rootParameters[1];
		//rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);

		//CD3DX12_DESCRIPTOR_RANGE1 ranges0[1]; // Only Texture
		//ranges0[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

		//CD3DX12_DESCRIPTOR_RANGE1 ranges1[2]; // Texture, Constant Buffer
		//ranges1[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
		//ranges1[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

		//CD3DX12_ROOT_PARAMETER1 rootParameters[TABLE_TYPE_END];
		//rootParameters[TEX].InitAsDescriptorTable(_countof(ranges0), ranges0, D3D12_SHADER_VISIBILITY_ALL);
		//rootParameters[TEX_CB].InitAsDescriptorTable(_countof(ranges1), ranges1, D3D12_SHADER_VISIBILITY_ALL);
		/*CD3DX12_ROOT_PARAMETER1 rootParameters[TABLE_TYPE_END];
		rootParameters[TEX].InitAsDescriptorTable(_countof(ranges0), ranges0, D3D12_SHADER_VISIBILITY_ALL);
		rootParameters[TEX_CB].InitAsDescriptorTable(_countof(ranges1), ranges1, D3D12_SHADER_VISIBILITY_ALL);*/



		CD3DX12_DESCRIPTOR_RANGE1 ranges[2]; // Texture, Constant Buffer
		ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
		ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

		CD3DX12_DESCRIPTOR_RANGE1 range0;
		range0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

		CD3DX12_DESCRIPTOR_RANGE1 range1[2];
		range1[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC); // Obj
		range1[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC); // VP_Light
		//range1[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 2, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
		
		CD3DX12_ROOT_PARAMETER1 rootParameters[2];
		rootParameters[0].InitAsDescriptorTable(1, &range0, D3D12_SHADER_VISIBILITY_PIXEL);
		rootParameters[1].InitAsDescriptorTable(_countof(range1), range1, D3D12_SHADER_VISIBILITY_ALL);


		D3D12_STATIC_SAMPLER_DESC sampler = {};
		sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
		sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		sampler.MipLODBias = 0;
		sampler.MaxAnisotropy = 0;
		sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
		sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
		sampler.MinLOD = 0.0f;
		sampler.MaxLOD = D3D12_FLOAT32_MAX;
		sampler.ShaderRegister = 0;
		sampler.RegisterSpace = 0;
		sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		ComPtr<ID3DBlob> signature;
		ComPtr<ID3DBlob> error;

		hr = D3DX12SerializeVersionedRootSignature(
			&rootSignatureDesc,
			RSFeatureData.HighestVersion,
			&signature,
			&error);
		if (FAILED(hr)) { return hr; }

		hr = m_pDevice->CreateRootSignature(
			0,
			signature->GetBufferPointer(),
			signature->GetBufferSize(),
			IID_PPV_ARGS(&m_rootSigArr[ROOTSIG_DEFAULT]));
		if (FAILED(hr)) { return hr; }
	}
#pragma endregion


#pragma endregion

#pragma region Make InputLayout
	{

		D3D12_INPUT_ELEMENT_DESC inputLayoutDesc_single[1][1]
		{ "POSITION",		0,		DXGI_FORMAT_R32G32B32_FLOAT,		0,		0,		D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,		0 };

		D3D12_INPUT_ELEMENT_DESC inputLayoutDesc_Double[2][2]
		{
			{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
			},
		 {
			 { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
			}
		};
		UINT iInputLayoutIndex = 0;
		// Vertex Type Single
		// m_vecInputLayoutArr[SHADERTYPE_SIMPLE].push_back(inputLayoutDesc_single[1][1]);

		// Vertex Type Double
		for (auto& iterDesc : inputLayoutDesc_Double[0])
		{
			m_vecInputLayoutArr[SHADERTYPE_SIMPLE].push_back(iterDesc);
		}
		for (auto& iterDesc : inputLayoutDesc_Double[1])
		{
			m_vecInputLayoutArr[SHADERTYPE_SIMPLE2].push_back(iterDesc);
		}

		//for (UINT iDoubleTypeIndex = 0; iDoubleTypeIndex < 2; ++iDoubleTypeIndex)
		//{
		//	for (auto& iterInputLayout : inputLayoutDesc_Double[iDoubleTypeIndex])
		//	{
		//		m_vecInputLayoutArr
		//	}
		//}
		//for (UINT iInputLayoutType/*VertexTypes*/ = 0; iInputLayoutType < RENDER_PARAMCOMBO_END; ++iInputLayoutType)
		//{
		//	for (auto& iter : input_layout_desc[iInputLayoutType])
		//	{
		//		m_vecInputLayoutArr[iInputLayoutType].push_back(iter);
		//	}
		//}
	}


#pragma endregion



#pragma region Build PSO
	D3D12_GRAPHICS_PIPELINE_STATE_DESC pso_desc{};
	pso_desc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	//pso_desc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
	//pso_desc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	pso_desc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	pso_desc.DepthStencilState.DepthEnable = FALSE;
	pso_desc.DepthStencilState.StencilEnable = FALSE;
	pso_desc.SampleMask = UINT_MAX; // 그 어떤 표본도 비활성화 하지 않음
	pso_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	pso_desc.NumRenderTargets = 1;
	pso_desc.RTVFormats[0] = m_pGraphic_Device->m_BackBufferFormat;
	pso_desc.SampleDesc.Count = 1;
	pso_desc.SampleDesc.Quality = 0;
	pso_desc.DSVFormat = m_pGraphic_Device->m_DepthStencilFormat;

	for (UINT IsFirst = 0; IsFirst < RENDER_PRIORITY_END; ++IsFirst)
	{
		for (UINT eBlendModeEnum = 0; eBlendModeEnum < RENDER_BLENDMODE_END; ++eBlendModeEnum)
		{
			for (UINT eShaderTypeEnum = 0; eShaderTypeEnum < RENDER_SHADERTYPE_END; ++eShaderTypeEnum)
			{
				wstring strKey = L"";
				switch (eShaderTypeEnum)
				{
				case SHADERTYPE_SIMPLE:
					strKey = L"Shader_Simple";
					break;
				case SHADERTYPE_SIMPLE2:
					strKey = L"Shader_Simple2";
					break;
				case SHADERTYPE_SIMPLE3:
					strKey = L"Shader_Simple3";
					break;
				default: // 에러 피하기 위해 임시로 만들어놓기
					strKey = L"Shader_Simple";
				}

				// VertexShader ByteCode
				CShader* pShader = dynamic_cast<CShader*>(m_pComponentManager->FindandGet_Prototype(strKey));
				/* 일단 shader type과 input layout 타입 일치시킴, 변경 가능성 */
				//pso_desc.InputLayout = { m_vecInputLayoutArr[eShaderTypeEnum].data(), (UINT)m_vecInputLayoutArr[eShaderTypeEnum].size() };
				pso_desc.InputLayout = { *(pShader->Get_InputLayoutArr()), pShader->Get_InputLayoutSize() };

				ComPtr<ID3DBlob>  byteCode = pShader->Get_ByteCode(CShader::TYPE_VERTEX);
				pso_desc.VS = CD3DX12_SHADER_BYTECODE(byteCode.Get());
				/*pso_desc.VS =
				{
					reinterpret_cast<BYTE*>(byteCode->GetBufferPointer()),
					byteCode->GetBufferSize()
				};*/
				byteCode = pShader->Get_ByteCode(CShader::TYPE_PIXEL);
				pso_desc.PS = CD3DX12_SHADER_BYTECODE(byteCode.Get());

				for (UINT eRootSigType = 0; eRootSigType < ROOTSIG_TYPE_END; ++eRootSigType)
				{
					pso_desc.pRootSignature = m_rootSigArr[eRootSigType];
					hr = m_pDevice->CreateGraphicsPipelineState(&pso_desc, IID_PPV_ARGS(&m_PSOsArr[IsFirst][eBlendModeEnum][eShaderTypeEnum][eRootSigType]));
					if (FAILED(hr))
					{
						MSG_BOX("Failed to Create PSO");
						return hr;
					}
				}
			}
		}
	}


	
#pragma endregion


	return hr;
}
HRESULT CPipelineManager::Free()
{
	for (auto& iter0 : m_PSOsArr)
	{
		for (auto& iter1 : iter0)
		{
			for (auto& iter2 : iter1)
			{
				for (auto& iter3 : iter2)
				{
					Safe_Release(iter3);
				}
			}
		}
	}

	for (auto& iter : m_rootSigArr)
	{
		Safe_Release(iter);
	}

	/*Safe_Release(m_pGameObjectManager);
	Safe_Release(m_pComponentManager);*/
	Safe_Release(m_pGraphic_Device);
	return S_OK;
}


ID3D12RootSignature* CPipelineManager::Get_RootSig(UINT eRootSigType)
{
	if (eRootSigType < 0 || eRootSigType >= ROOTSIG_TYPE_END)
	{
		return nullptr;
	}

	return m_rootSigArr[eRootSigType];
}


ID3D12PipelineState* CPipelineManager::Get_PSO(UINT IsFirst, UINT eBlendModeEnum, UINT eShaderTypeEnum, UINT eRootsigType)
{
	return m_PSOsArr[IsFirst][eBlendModeEnum][eShaderTypeEnum][eRootsigType];
}

void CPipelineManager::Update_ObjPipelineLayer(CGameObject* pObject, ENUM_PSO ePsoEnum)
{
	for (auto& listIter : m_vecPipelineLayerArr)
	{
		for (auto& objIter = listIter.begin(); objIter != listIter.end();)
		{
			
			if (*objIter == pObject)
			{
				// 새로 넣어주고 기존꺼 지워주기
				m_vecPipelineLayerArr[ePsoEnum].push_back(pObject);
				objIter = listIter.erase(objIter);
				return;
			}
			else
			{
				++objIter;
			}
		}
	}
	// 못 찾았으면 (아예 처음 호출) 넣어주기
	m_vecPipelineLayerArr[ePsoEnum].push_back(pObject);
}
