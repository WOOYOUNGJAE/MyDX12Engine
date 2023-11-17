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

	return S_OK;
}

HRESULT CPipeline::Free()
{
	Safe_Release(m_pGraphic_Device);
	return S_OK;
}

