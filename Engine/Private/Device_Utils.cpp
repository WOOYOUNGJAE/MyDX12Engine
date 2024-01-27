#include "Device_Utils.h"

HRESULT CDevice_Utils::Create_Buffer_Default(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList,
                                             const void* initData, UINT64 byteSize, ComPtr<ID3D12Resource>& uploadBuffer,
                                             ComPtr<ID3D12Resource>& refOutResource)
{
    ComPtr<ID3D12Resource> defaultBuffer;

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
        IID_PPV_ARGS(defaultBuffer.GetAddressOf()));

    if (FAILED(hr)) { return E_FAIL; }

    // GPU�� ���ۿ� ���翡 �غ� �ʿ��� �ӽ� ���ε� �� ����
    temp_heap_properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    hr = device->CreateCommittedResource(
        &temp_heap_properties,
        D3D12_HEAP_FLAG_NONE,
        &TempBuffer,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(uploadBuffer.GetAddressOf()));

    if (FAILED(hr)) { return E_FAIL; }

    // �⺻ ���ۿ� ������ �ڷ� ����
    D3D12_SUBRESOURCE_DATA subResourceData = {};
    subResourceData.pData = initData;
    // ���ۿ� �����ؼ� ������ �ڷ��� ũ�� (����Ʈ ����)�� ����
    subResourceData.RowPitch = byteSize;
    subResourceData.SlicePitch = subResourceData.RowPitch;

    CD3DX12_RESOURCE_BARRIER tempBarrier = CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(),
        D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
    cmdList->ResourceBarrier(1, &tempBarrier);

    // �⺻ ���� �ڿ������� �ڷ� ���� ��û
    // UpdateSubresource : CPU�޸𸮸� �ӽ� ���ε� ���� �����ϰ� ID3D12CommandList::CopySubresourceRegion�� �ӽ� ���ε� ���� �ڷḦ buffer�� ����
    UpdateSubresources<1>(cmdList, defaultBuffer.Get(), uploadBuffer.Get(), 0, 0, 1, &subResourceData);
    //
    tempBarrier = CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(),
        D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
    cmdList->ResourceBarrier(1, &tempBarrier);

    refOutResource = defaultBuffer; // 

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
