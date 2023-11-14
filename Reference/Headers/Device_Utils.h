#pragma once
#include "Engine_Defines.h"
class CDevice_Utils
{
private:
	CDevice_Utils() = default;
	~CDevice_Utils() = default;

public:
	static HRESULT Create_Buffer_Default(
		ID3D12Device* device,
		ID3D12GraphicsCommandList* cmdList,
		const void* initData,
		UINT64 byteSize,
		Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer,
		Microsoft::WRL::ComPtr<ID3D12Resource>& refOutResource);	

};

