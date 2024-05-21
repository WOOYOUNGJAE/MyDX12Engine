#include "Device_Utils.h"

void DXR_Util::Build_BLAS(ID3D12Device5* pDevice, ID3D12GraphicsCommandList4* pCommandList, DXR::BLAS* pBLAS,
	ID3D12Resource* pIndexBuffer, ID3D12Resource* pVertexBuffer, DXGI_FORMAT IndexFormat, UINT iTriangleIndexCount, UINT iTriangleVertexCount, UINT64 iVertexStrideInBytes)
{
	pBLAS->indexBuffer = pIndexBuffer;
	pBLAS->vertexBuffer = pVertexBuffer;

	// DXR Geometry Desc
	D3D12_RAYTRACING_GEOMETRY_DESC dxrGeometryDesc{};
	pBLAS->dxrGeometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
	pBLAS->dxrGeometryDesc.Triangles.Transform3x4 = 0;
	pBLAS->dxrGeometryDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE; // 일단 OPAQUE

	pBLAS->dxrGeometryDesc.Triangles.IndexFormat = IndexFormat;
	pBLAS->dxrGeometryDesc.Triangles.IndexCount = iTriangleIndexCount;
	pBLAS->dxrGeometryDesc.Triangles.IndexBuffer = pBLAS->indexBuffer->GetGPUVirtualAddress();

	pBLAS->dxrGeometryDesc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT; // Fixed
	pBLAS->dxrGeometryDesc.Triangles.VertexCount = iTriangleVertexCount;
	pBLAS->dxrGeometryDesc.Triangles.VertexBuffer.StartAddress = pBLAS->vertexBuffer->GetGPUVirtualAddress();
	pBLAS->dxrGeometryDesc.Triangles.VertexBuffer.StrideInBytes = iVertexStrideInBytes;

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS& refBottomLevelInputs = pBLAS->accelerationStructureDesc.Inputs;
	refBottomLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	refBottomLevelInputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
	refBottomLevelInputs.NumDescs = 1;
	refBottomLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
	refBottomLevelInputs.pGeometryDescs = &(pBLAS->dxrGeometryDesc);

	pDevice->GetRaytracingAccelerationStructurePrebuildInfo(&pBLAS->accelerationStructureDesc.Inputs, &pBLAS->prebuildInfo);
	if (pBLAS->prebuildInfo.ResultDataMaxSizeInBytes == 0)
	{
		MSG_BOX("MeshData : Building BLAS Failed");
		return;
	}

	// Allocate Scratch Buffer if cur Blas ResultDataMaxSizeInBytes is Bigger;
	::DXR_Util::AllocateScratch_IfBigger(pDevice, pBLAS->prebuildInfo.ResultDataMaxSizeInBytes);

	// Allocate UAV Buffer, 실질적인 BLAS Allocate
	HRESULT hr = MyUtils::AllocateUAVBuffer(pDevice,
		pBLAS->prebuildInfo.ResultDataMaxSizeInBytes,
		&pBLAS->uav_BLAS,
		D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE);
	if (FAILED(hr)) { MSG_BOX("Allocate Failed"); }

	pBLAS->accelerationStructureDesc.ScratchAccelerationStructureData = (*CDXRResource::Get_Instance()->Get_ScratchBufferPtr())->GetGPUVirtualAddress();
	pBLAS->accelerationStructureDesc.DestAccelerationStructureData = pBLAS->uav_BLAS->GetGPUVirtualAddress();

	pCommandList->BuildRaytracingAccelerationStructure(&pBLAS->accelerationStructureDesc, 0, nullptr);
	pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(pBLAS->uav_BLAS));
}
