#include "Pipeline.h"

#include "ComponentManager.h"
#include "GameObject.h"
#include "Shader.h"
#include "Graphic_Device.h"
#include "Transform.h"

IMPLEMENT_SINGLETON(CPipeline)

CPipeline::CPipeline() : m_pComponentManager(CComponentManager::Get_Instance())
{
	Safe_AddRef(m_pComponentManager);
}

HRESULT CPipeline::Initialize()
{
	// GraphicDevice들의 멤버들이 초기화 된 후 참조
	m_pGraphic_Device = CGraphic_Device::Get_Instance();
	m_pDevice = m_pGraphic_Device->Get_Device();
	m_pCommandList= m_pGraphic_Device->Get_CommandList();
	Safe_AddRef(m_pGraphic_Device);

	// Create ConstantBufferView Descriptor Heap
	{
		D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
		cbvHeapDesc.NumDescriptors = 1;
		cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		cbvHeapDesc.NodeMask = 0;

		if (FAILED(m_pDevice->CreateDescriptorHeap(&cbvHeapDesc,
			IID_PPV_ARGS(&m_pCbvHeap))))
		{
			return E_FAIL;
		}
	}

	if (FAILED(Init_ConstantBuffers()))
	{
		return E_FAIL;
	}

	if (FAILED(Init_RootSignature()))
	{
		return E_FAIL;
	}

	// Build PSO
	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC pso_desc{};
		pso_desc.InputLayout = { m_vecInputLayout[InputLayout_DEFAULT].data(), (UINT)m_vecInputLayout[InputLayout_DEFAULT].size() };
		pso_desc.pRootSignature = m_RootSig[RootSig_DEFAULT].Get();
		// VertexShader ByteCode
		ComPtr<ID3DBlob>  byteCode = dynamic_cast<CShader*>(m_pComponentManager->Find_Prototype(L"vShader_Default"))->Get_ByteCode(CShader::TYPE_VERTEX);
		pso_desc.VS =
		{
			reinterpret_cast<BYTE*>(byteCode->GetBufferPointer()),
			byteCode->GetBufferSize()
		};
		byteCode = dynamic_cast<CShader*>(m_pComponentManager->Find_Prototype(L"pShader_Default"))->Get_ByteCode(CShader::TYPE_PIXEL);
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

		if (FAILED(Build_PSO(pso_desc, ENUM_PSO::PSO_DEFAULT)))
		{
			MSG_BOX("Pipeline : Build PSO Failed ");
			return E_FAIL;
		}
		
	}
	return S_OK;
}

HRESULT CPipeline::Init_ConstantBuffers()
{
	m_pUploadBuffer_Constant = CUploadBuffer<_float4x4>::Create(m_pDevice.Get(), 1, true);

	if (m_pUploadBuffer_Constant == nullptr)
	{
		return E_FAIL;
	}

	UINT objCBByteSize = CDevice_Utils::ConstantBufferByteSize(sizeof(_float4x4));

	// 0번째 상수 버퍼의 시작 주소
	D3D12_GPU_VIRTUAL_ADDRESS cbAddress = m_pUploadBuffer_Constant->Get_UploadBuffer()->GetGPUVirtualAddress();
	// Offset to the ith object constant buffer in the buffer.
	// 버퍼에 담긴 i번째 상수 버퍼의 오프셋
	int boxCBufIndex = 0;
	cbAddress += boxCBufIndex * objCBByteSize;


	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
	cbvDesc.BufferLocation = cbAddress;
	cbvDesc.SizeInBytes = CDevice_Utils::ConstantBufferByteSize(sizeof(_float4x4));

	m_pGraphic_Device->m_pCommandList;

	m_pDevice->CreateConstantBufferView(
		&cbvDesc,
		m_pCbvHeap->GetCPUDescriptorHandleForHeapStart());

	return S_OK;
}

HRESULT CPipeline::Init_RootSignature()
{
	// TODO RootSig 서술자 테이블 2개로 사용하기 -> default?
	// RootSig_DEFAULT

	// 루트 시그니쳐는 테이블이거나 루트 서술자 또는 루트 상수이다.
	CD3DX12_ROOT_PARAMETER slotRootParameter[1];

	// CBV 하나를 담는 서술자 테이블 생성
	CD3DX12_DESCRIPTOR_RANGE cbvTable;
	cbvTable.Init(
		D3D12_DESCRIPTOR_RANGE_TYPE_CBV,
		1, // 테이블의 서술자 개수
		0
	);
	slotRootParameter[0].InitAsDescriptorTable(1, &cbvTable);

	// 루트 시그니쳐는 루트 매개변수들의 배열
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(
		1, 
		slotRootParameter, 
		0, 
		nullptr,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	// 상수 버퍼 하나로 구성된 서술자 구간을 가리키는, 슬롯하나로 이루어진 루트 시그니쳐 생성
	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;

	if (FAILED(D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf())))
	{
		return E_FAIL;
	}

	if (errorBlob)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}

	if (FAILED(m_pDevice->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(&m_RootSig[RootSig_DEFAULT]))))
	{
		return E_FAIL;
	}

	return S_OK;
}

void CPipeline::Pipeline_Tick()
{
	for (auto& iter : m_vecPipelineLayerArr)
	{
		for (auto& innerIter : iter)
		{
			innerIter->Pipeline_Tick();
		}
	}
}

HRESULT CPipeline::Free()
{
	Safe_Release(m_pComponentManager);
	Safe_Release(m_pUploadBuffer_Constant);
	Safe_Release(m_pGraphic_Device);
	return S_OK;
}

HRESULT CPipeline::Build_PSO(const D3D12_GRAPHICS_PIPELINE_STATE_DESC& pipeline_desc, ENUM_PSO psoIndex)
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC pso_desc = pipeline_desc;
	if (FAILED(m_pDevice->CreateGraphicsPipelineState(&pso_desc, IID_PPV_ARGS(&(m_PSOArr[psoIndex])))))
	{
		MSG_BOX("Failed to Create PSO");
		return E_FAIL;
	}

	return S_OK;
}
