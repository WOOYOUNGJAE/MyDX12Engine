#pragma once
#include "Engine_Defines.h"
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
		ComPtr<ID3D12Resource>& uploadBuffer,
		ComPtr<ID3D12Resource>& refOutResource);

	// 256�� �ּ� ������� ���
	static _uint ConstantBufferByteSize(_uint iByteSize)
	{
		return (iByteSize + 255) & ~255;
	}

};

