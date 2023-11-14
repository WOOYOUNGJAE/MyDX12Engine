#pragma once
#include "Component.h"
#include "Engine_Defines.h"
class CMeshGeometry abstract: CComponent
{
protected:
	CMeshGeometry() = default;
	~CMeshGeometry() override = default;

public:
	virtual HRESULT Free() override;

protected:
	virtual HRESULT Create_Buffer(ID3D12GraphicsCommandList* cmdList,
		const void* initData, UINT64 byteSize, Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer,
		Microsoft::WRL::ComPtr<ID3D12Resource>* pOutResource);

protected:
	//std::array<>
};

