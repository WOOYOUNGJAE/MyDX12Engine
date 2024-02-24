#pragma once
#include "Engine_Defines.h"

NAMESPACE_(Engine)

class CDevice_Utils
{
private:
	CDevice_Utils() = default;
	~CDevice_Utils() = default;

public:
	/// ���ؽ�, �ε��� ���� ��� ����
	static HRESULT Create_Buffer_Default(
		ID3D12Device* device,
		ID3D12GraphicsCommandList* cmdList,
		const void* initData,
		UINT64 byteSize,
		ID3D12Resource** ppUploadBuffer,
		ID3D12Resource** ppOutResource);

	// 256�� �ּ� ������� ���
	static _uint ConstantBufferByteSize(_uint iByteSize)
	{
		return (iByteSize + 255) & ~255;
	}

	static ComPtr<ID3DBlob> CompileShader(
		const std::wstring& filename,
		const D3D_SHADER_MACRO* defines,
		const std::string& entrypoint,
		const std::string& target);

};

_NAMESPACE