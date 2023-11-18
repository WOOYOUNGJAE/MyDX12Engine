#include "Pipeline.h"
#include "Graphic_Device.h"

IMPLEMENT_SINGLETON(CPipeline)

CPipeline::CPipeline() :
	m_pGraphic_Device(CGraphic_Device::Get_Instance()),
	m_pDevice(m_pGraphic_Device->Get_Device()),
	m_pCommandList(m_pGraphic_Device->Get_CommandList())
{
	Safe_AddRef(m_pGraphic_Device);
}

HRESULT CPipeline::Initialize()
{
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

HRESULT CPipeline::Free()
{
	Safe_Release(m_pGraphic_Device);
	return S_OK;
}

