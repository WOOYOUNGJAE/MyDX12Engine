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

	if (RootSig_Exist(L"Triangle") == false)
	{
		Add_NewRootSig(L"Triangle", nullptr);
	}

	hr = m_pDevice->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(&m_mapRootSig[L"Triangle"]));
	if (FAILED(hr)) { return hr; }

	/*if (FAILED(m_pDevice->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(n))))
	{
		return E_FAIL;
	}*/


	//// CBV 하나를 담는 서술자 테이블 생성, 현재 Default는 ConstantObject, ConstantPass로 총 두개
	//CD3DX12_DESCRIPTOR_RANGE cbvTable0;
	//cbvTable0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	//CD3DX12_DESCRIPTOR_RANGE cbvTable1;
	//cbvTable1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1/*인덱스*/);

	//// 루트 시그니쳐는 테이블이거나 루트 서술자 또는 루트 상수
	//CD3DX12_ROOT_PARAMETER slotRootParameterArr[2];

	//slotRootParameterArr[0].InitAsDescriptorTable(1, &cbvTable0);
	//slotRootParameterArr[1].InitAsDescriptorTable(1, &cbvTable1);

	//// 루트 시그니쳐는 루트 매개변수들의 배열
	//CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(
	//	2,
	//	slotRootParameterArr,
	//	0,
	//	nullptr,
	//	D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	//// 상수 버퍼 하나로 구성된 서술자 구간을 가리키는, 슬롯하나로 이루어진 루트 시그니쳐 생성
	//ComPtr<ID3DBlob> serializedRootSig = nullptr;
	//ComPtr<ID3DBlob> errorBlob = nullptr;

	//hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
	//	serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());
	//if (FAILED(hr)) { return hr; }

	//if (errorBlob)
	//{
	//	::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	//}

	/*if (FAILED(m_pDevice->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(n))))
	{
		return E_FAIL;
	}*/

#pragma endregion


#pragma region Make InputLayout
	D3D12_INPUT_ELEMENT_DESC input_layout_desc[2]
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
	
	for (auto& iter : input_layout_desc)
	{
		//pPipelineManager->Push_NewInputLayout(iter, CPipelineManager::InputLayout_DEFAULT);;
		Push_NewInputLayout(L"Default", iter);
	}
#pragma endregion



#pragma region Build PSO
	D3D12_GRAPHICS_PIPELINE_STATE_DESC pso_desc{};
	pso_desc.InputLayout = { m_mapVecInputLayout[L"Default"].data(), (UINT)m_mapVecInputLayout[L"Default"].size() };
	pso_desc.pRootSignature = m_mapRootSig[L"Triangle"];
	// VertexShader ByteCode
	CShader* pShader = dynamic_cast<CShader*>(m_pComponentManager->Find_Prototype(L"Shader_Simple"));
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
	pso_desc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	pso_desc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	pso_desc.SampleMask = UINT_MAX; // 그 어떤 표본도 비활성화 하지 않음
	pso_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	pso_desc.NumRenderTargets = 1;
	pso_desc.RTVFormats[0] = m_pGraphic_Device->m_BackBufferFormat;
	pso_desc.SampleDesc.Count = 1;
	pso_desc.SampleDesc.Quality = 0;
	pso_desc.DSVFormat = m_pGraphic_Device->m_DepthStencilFormat;

	if (PSO_Exist(L"Triangle") == false)
	{
		Add_NewPSO(L"Triangle", nullptr);
	}

	hr = Build_PSO(L"Triangle", pso_desc);
	if (FAILED(hr))
	{
		MSG_BOX("Pipeline : Build PSO Failed ");
		return hr;
	}
#pragma endregion


	return hr;
}

void CPipelineManager::Pipeline_Tick(_float fDeltaTime)
{
	for (auto& iter : m_vecPipelineLayerArr)
	{
		for (auto& innerIter : iter)
		{
			innerIter->Pipeline_Tick();
		}
	}
}



void CPipelineManager::Render()
{
	// Renderer가 대체
}
HRESULT CPipelineManager::Free()
{
	for (auto& iter : m_mapPSO)
	{
		iter.second->Release();
	}

	for (auto& iter : m_mapRootSig)
	{
		iter.second->Release();
	}

	/*Safe_Release(m_pGameObjectManager);
	Safe_Release(m_pComponentManager);*/
	Safe_Release(m_pGraphic_Device);
	return S_OK;
}

HRESULT CPipelineManager::Build_PSO(const D3D12_GRAPHICS_PIPELINE_STATE_DESC& pipeline_desc, ENUM_PSO psoIndex)
{
	HRESULT hr = S_OK;
	D3D12_GRAPHICS_PIPELINE_STATE_DESC pso_desc = pipeline_desc;
	hr = m_pDevice->CreateGraphicsPipelineState(&pso_desc, IID_PPV_ARGS(&(m_PSOArr[psoIndex])));
	if (FAILED(hr))
	{
		MSG_BOX("Failed to Create PSO");
		return hr;
	}

	return hr;
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

void CPipelineManager::Push_NewInputLayout(const wstring& strKey, D3D12_INPUT_ELEMENT_DESC desc)
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
