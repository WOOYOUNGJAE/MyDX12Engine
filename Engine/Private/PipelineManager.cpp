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

_uint g_iNumFrameResource = 3; // extern frame resource num;
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
	// RootSig - Triangle Sample
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc;
	rootSigDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	// 상수 버퍼 하나로 구성된 서술자 구간을 가리키는, 슬롯하나로 이루어진 루트 시그니쳐 생성
	ComPtr<ID3DBlob> serializedRootSig;
	ComPtr<ID3DBlob> errorBlob;

	hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());
	if (FAILED(hr)) { return hr; }

	if (errorBlob)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}



	/*if (RootSig_Exist(L"Triangle") == false)
	{
		Add_NewRootSig(L"Triangle", nullptr);
	}*/

	hr = m_pDevice->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(&m_rootSigArr[ROOTSIG_DEFAULT]));
	if (FAILED(hr)) { return hr; }

#pragma endregion


#pragma region Make InputLayout
	D3D12_INPUT_ELEMENT_DESC input_layout_desc[2]
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
	
	for (auto& iter : input_layout_desc)
	{
		//pPipelineManager->Register_NewInputLayout(iter, CPipelineManager::InputLayout_DEFAULT);;
		Register_NewInputLayout(L"Default", iter);
	}
#pragma endregion



#pragma region Build PSO
	D3D12_GRAPHICS_PIPELINE_STATE_DESC pso_desc{};
	pso_desc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	pso_desc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	pso_desc.SampleMask = UINT_MAX; // 그 어떤 표본도 비활성화 하지 않음
	pso_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	pso_desc.NumRenderTargets = 1;
	pso_desc.RTVFormats[0] = m_pGraphic_Device->m_BackBufferFormat;
	pso_desc.SampleDesc.Count = 1;
	pso_desc.SampleDesc.Quality = 0;
	pso_desc.DSVFormat = m_pGraphic_Device->m_DepthStencilFormat;
	pso_desc.InputLayout = { m_mapVecInputLayout[L"Default"].data(), (UINT)m_mapVecInputLayout[L"Default"].size() };

	for (UINT IsFirst = 0; IsFirst < RENDER_PRIORITY_END; ++IsFirst)
	{
		for (UINT eBlendModeEnum = 0; eBlendModeEnum < RENDER_BLENDMODE_END; ++eBlendModeEnum)
		{
			for (UINT eRootsigEnum = 0; eRootsigEnum < RENDER_ROOTSIGTYPE_END; ++eRootsigEnum)
			{
				pso_desc.pRootSignature = m_rootSigArr[eRootsigEnum];

				for (UINT eShaderTypeEnum = 0; eShaderTypeEnum < RENDER_SHADERTYPE_END; ++eShaderTypeEnum)
				{
					wstring strKey = L"";
					switch (eShaderTypeEnum)
					{
					case SHADERTYPE_SIMPLE:
						strKey = L"Shader_Simple";
						break;
					default: // 에러 피하기 위해 임시로 만들어놓기
						strKey = L"Shader_Simple";
					}

					// VertexShader ByteCode
					CShader* pShader = dynamic_cast<CShader*>(m_pComponentManager->FindandGet_Prototype(strKey));
					ComPtr<ID3DBlob>  byteCode = pShader->Get_ByteCode(CShader::TYPE_VERTEX);
					pso_desc.VS =
					{
						reinterpret_cast<BYTE*>(byteCode->GetBufferPointer()),
						byteCode->GetBufferSize()
					};
					byteCode = pShader->Get_ByteCode(CShader::TYPE_PIXEL);
					pso_desc.PS =
					{
						reinterpret_cast<BYTE*>(byteCode->GetBufferPointer()),
						byteCode->GetBufferSize()
					};

					hr = m_pDevice->CreateGraphicsPipelineState(&pso_desc, IID_PPV_ARGS(&m_PSOsArr[IsFirst][eBlendModeEnum][eRootsigEnum][eShaderTypeEnum]));
					if (FAILED(hr))
					{
						MSG_BOX("Failed to Create PSO");
						return hr;
					}
				}
			}
		}
	}

	

	/*if (PSO_Exist(L"Triangle") == false)
	{
		Add_NewPSO(L"Triangle", nullptr);
	}*/

	/*hr = Build_PSO(L"Triangle", pso_desc);
	if (FAILED(hr))
	{
		MSG_BOX("Pipeline : Build PSO Failed ");
		return hr;
	}*/
#pragma endregion


	return hr;
}

HRESULT CPipelineManager::Free()
{
	for (UINT IsFirst = 0; IsFirst < RENDER_PRIORITY_END; ++IsFirst)
	{
		for (UINT eBlendModeEnum = 0; eBlendModeEnum < RENDER_BLENDMODE_END; ++eBlendModeEnum)
		{
			for (UINT eRootsigEnum = 0; eRootsigEnum < RENDER_ROOTSIGTYPE_END; ++eRootsigEnum)
			{
				for (UINT eShaderTypeEnum = 0; eShaderTypeEnum < RENDER_SHADERTYPE_END; ++eShaderTypeEnum)
				{
					Safe_Release(m_PSOsArr[IsFirst][eBlendModeEnum][eRootsigEnum][eShaderTypeEnum]);					
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


HRESULT CPipelineManager::Build_PSO(const wstring& strKey, const D3D12_GRAPHICS_PIPELINE_STATE_DESC& pipeline_desc)
{
	HRESULT hr = S_OK;
	D3D12_GRAPHICS_PIPELINE_STATE_DESC pso_desc = pipeline_desc;
	hr = m_pDevice->CreateGraphicsPipelineState(&pso_desc, IID_PPV_ARGS(&(m_mapPSO[strKey])));
	if (FAILED(hr))
	{
		MSG_BOX("Failed to Create PSO");
		return hr;
	}

	return hr;
}

void CPipelineManager::Register_NewInputLayout(const wstring& strKey, D3D12_INPUT_ELEMENT_DESC desc)
{
	// 새로 만들어야 할 때
	if (m_mapVecInputLayout.find(strKey) == m_mapVecInputLayout.end())
	{
		m_mapVecInputLayout.emplace(strKey, vector<D3D12_INPUT_ELEMENT_DESC>());
	}

	m_mapVecInputLayout[strKey].push_back(desc);
}


ID3D12RootSignature* CPipelineManager::Get_RootSig(const wstring& strKey)
{
	if (RootSig_Exist(strKey))
	{
		return m_mapRootSig[strKey];
	}
	else
	{
		return nullptr;
	}
}

ID3D12RootSignature* CPipelineManager::Get_RootSig(UINT eRootSigType)
{
	if (eRootSigType < 0 || eRootSigType >= RENDER_ROOTSIGTYPE_END)
	{
		return nullptr;
	}

	return m_rootSigArr[eRootSigType];
}

ID3D12PipelineState* CPipelineManager::Get_PSO(const wstring& strKey)
{
	if (PSO_Exist(strKey))
	{
		return m_mapPSO[strKey];
	}
	else
	{
		return nullptr;
	}
}

ID3D12PipelineState* CPipelineManager::Get_PSO(UINT IsFirst, UINT eBlendModeEnum, UINT eRootsigEnum, UINT eShaderTypeEnum)
{
	return m_PSOsArr[IsFirst][eBlendModeEnum][eRootsigEnum][eShaderTypeEnum];
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
