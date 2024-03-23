#include "Device_Utils.h"
#include "MeshData.h"
#include "MeshDataType.h"

HRESULT CDevice_Utils::Create_Buffer_Default(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList,
                                             const void* initData, UINT64 byteSize, ID3D12Resource** ppUploadBuffer,
                                             ID3D12Resource** ppOutResource)
{
    ID3D12Resource* defaultBuffer;

    CD3DX12_HEAP_PROPERTIES temp_heap_properties(D3D12_HEAP_TYPE_DEFAULT);
    CD3DX12_RESOURCE_DESC TempBuffer =
        CD3DX12_RESOURCE_DESC::Buffer (byteSize);

    // �⺻ ���� �ڿ� ����
    HRESULT hr = S_OK;
    hr = device->CreateCommittedResource(
        &temp_heap_properties,
        D3D12_HEAP_FLAG_NONE,
        &TempBuffer,
        D3D12_RESOURCE_STATE_COMMON,
        nullptr,
        IID_PPV_ARGS(&defaultBuffer));

    if (FAILED(hr)) { return E_FAIL; }

    // GPU�� ���ۿ� ���翡 �غ� �ʿ��� �ӽ� ���ε� �� ����
    temp_heap_properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    hr = device->CreateCommittedResource(
        &temp_heap_properties,
        D3D12_HEAP_FLAG_NONE,
        &TempBuffer,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(ppUploadBuffer));

    if (FAILED(hr)) { return E_FAIL; }

    // �⺻ ���ۿ� ������ �ڷ� ����
    D3D12_SUBRESOURCE_DATA subResourceData = {};
    subResourceData.pData = initData;
    // ���ۿ� �����ؼ� ������ �ڷ��� ũ�� (����Ʈ ����)�� ����
    subResourceData.RowPitch = byteSize;
    subResourceData.SlicePitch = subResourceData.RowPitch;

    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer,
        D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));

    // �⺻ ���� �ڿ������� �ڷ� ���� ��û
    // UpdateSubresource : CPU�޸𸮸� �ӽ� ���ε� ���� �����ϰ� ID3D12CommandList::CopySubresourceRegion�� �ӽ� ���ε� ���� �ڷḦ buffer�� ����
    UpdateSubresources<1>(cmdList, defaultBuffer, *ppUploadBuffer, 0, 0, 1, &subResourceData);
    //
    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer,
        D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));

    *ppOutResource = defaultBuffer; // 

    // �� �Լ� ȣ�� ���Ŀ��� uploadBuffer�� �����ؾ� ��
    // ���簡 �Ϸ�Ǿ����� Ȯ������ �Ŀ� �����ϸ� ��

	return hr;
}

ComPtr<ID3DBlob> CDevice_Utils::CompileShader(const std::wstring& filename, const D3D_SHADER_MACRO* defines,
	const std::string& entrypoint, const std::string& target)
{
    UINT compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)  
    compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    HRESULT hr = S_OK;

    ComPtr<ID3DBlob> byteCode = nullptr;
    ComPtr<ID3DBlob> errors;

    hr = D3DCompileFromFile(filename.c_str(), defines, D3D_COMPILE_STANDARD_FILE_INCLUDE,
        entrypoint.c_str(), target.c_str(), compileFlags, 0, &byteCode, &errors);


    if (FAILED(hr))
    {
        MSG_BOX("Failed to Compile Shader");

	    if (errors != nullptr)
	        OutputDebugStringA((char*)errors->GetBufferPointer());
    }


    return byteCode;
}

D3D12_RAYTRACING_GEOMETRY_DESC CDevice_Utils::Generate_AccelerationStructureDesc(CMeshData* pMeshData, ID3D12Resource* pSrvIndex, ID3D12Resource* pSrvVertex)
{
    D3D12_RAYTRACING_GEOMETRY_DESC geometryDesc = {};
    geometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES; // Fixed
    geometryDesc.Triangles.IndexBuffer = pSrvIndex->GetGPUVirtualAddress();
    geometryDesc.Triangles.IndexCount = pMeshData->Num_Indices();
    geometryDesc.Triangles.IndexFormat = pMeshData->Get_IndexFormat();
    geometryDesc.Triangles.Transform3x4 = 0;
    geometryDesc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT; // Fixed
    geometryDesc.Triangles.VertexCount = pMeshData->Num_Vertices();
    geometryDesc.Triangles.VertexBuffer.StartAddress = pSrvVertex->GetGPUVirtualAddress();
    geometryDesc.Triangles.VertexBuffer.StrideInBytes = pMeshData->Get_StrideInBytes();

    return geometryDesc;
}
